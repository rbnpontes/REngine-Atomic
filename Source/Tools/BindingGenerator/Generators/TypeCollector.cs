using System.Diagnostics;
using BindingGenerator.Models;
using BindingGenerator.Utils;
using CppAst;

namespace BindingGenerator.Generators;

public struct TypeCollectorCreateDesc
{
    public CppCompilation Compilation;
    public Module Module;
    public List<(ModuleItem, string)> ModuleItems;
}

public class TypeCollector(TypeCollectorCreateDesc createDesc)
{
    private Dictionary<string, (TypeDefinition, CppElement)> pTypes = new();
    private Dictionary<CppElement, TypeDefinition> pCppTypes = new();
    private HashSet<string> pAllowedClasses = new();
    
    public NamespaceDefinition Namespace { get; private set; } = new GlobalNamespaceDefinition();
    public int TotalOfTypes => pTypes.Count;

    public void Collect()
    {
        pTypes.Clear();
        pCppTypes.Clear();
        pAllowedClasses.Clear();
        
        Namespace = new GlobalNamespaceDefinition();

        var compilation = createDesc.Compilation;
        Dictionary<string, ModuleItem> allowedSrcFiles =
            new(createDesc.ModuleItems.Select(x => new KeyValuePair<string, ModuleItem>(x.Item2, x.Item1)));
        
        BuildAllowedClassesMap();
        
        CollectEnums(compilation.Enums, allowedSrcFiles, Namespace);
        CollectClasses(compilation.Classes, allowedSrcFiles, Namespace);
        CollectNamespaces(compilation.Namespaces, allowedSrcFiles, Namespace);
        
        // Method collection must occur after Class, Structs and Enums collection
        // Otherwise, null references will occur
        CollectStaticMethods(compilation.Functions, allowedSrcFiles, Namespace);
        CollectStaticMethodsFromNamespaces(Namespace.Namespaces, allowedSrcFiles);
        CollectClassMethods(Namespace);
    }

    private void CollectNamespaces(CppContainerList<CppNamespace> namespaces,
        IDictionary<string, ModuleItem> allowedSourceFiles, NamespaceDefinition @namespace)
    {
        var namespaceResult = new List<NamespaceDefinition>();
        foreach (var namespaceType in namespaces)
        {
            var namespaceDef = new NamespaceDefinition();
            namespaceDef.Owner = @namespace;
            namespaceDef.Name = namespaceType.Name;
            namespaceDef.Comment = namespaceType.Comment?.ToString() ?? string.Empty;

            CollectEnums(namespaceType.Enums, allowedSourceFiles, namespaceDef);
            CollectClasses(namespaceType.Classes, allowedSourceFiles, namespaceDef);

            // Only add if namespace is in use
            if (namespaceDef.IsEmpty)
                continue;
            
            namespaceResult.Add(namespaceDef);
            pTypes.TryAdd(namespaceDef.GetUniqueName(), (namespaceDef, namespaceType));
            pCppTypes.TryAdd(namespaceType, namespaceDef);
        }

        @namespace.Namespaces = namespaceResult.ToArray();
    }

    /**
     * We must have a special map called Allowed Class Map
     * This is required to exclude unused classes.
     * Only mapped classes will be included at full type definition.
     */
    private void BuildAllowedClassesMap()
    {
        createDesc.ModuleItems.ForEach(pair =>
        {
            var (moduleItem, _) = pair;
            foreach (var klass in moduleItem.Classes)
                pAllowedClasses.Add(klass);
        });
    }
    private void CollectEnums(CppContainerList<CppEnum> enums, IDictionary<string, ModuleItem> allowedSourceFiles,
        NamespaceDefinition @namespace)
    {
        var enumsResult = new List<EnumDefinition>();
        foreach (var @enum in enums)
        {
            if (string.IsNullOrEmpty(@enum.SourceFile))
                continue;
            // Only process mapped source files. Other modules will be skipped
            var srcFile = Path.GetFullPath(@enum.SourceFile);
            if (!allowedSourceFiles.TryGetValue(srcFile, out var moduleItem))
                continue;

            if(AstUtils.IsIgnoredType(@enum))
                continue;
            
            var enumDef = new EnumDefinition(@namespace)
            {
                Name = @enum.GetDisplayName(),
                Comment = @enum.Comment?.ToString() ?? string.Empty,
                HeaderFilePath = srcFile,
            };
            enumDef.Entries = @enum.Items
                .Select(enumItem => new EnumEntry(enumDef)
                {
                    Comment = enumItem.Comment?.ToString() ?? string.Empty,
                    Name = enumItem.Name,
                    Value = (int)enumItem.Value
                }).ToArray();

            enumsResult.Add(enumDef);
            pTypes.TryAdd(enumDef.GetUniqueName(), (enumDef, @enum));
            pCppTypes.TryAdd(@enum, enumDef);
        }

        @namespace.Enums = enumsResult.ToArray();
    }

    private void CollectClasses(CppContainerList<CppClass> classes,
        IDictionary<string, ModuleItem> allowedSourceFiles, NamespaceDefinition @namespace)
    {
        var classesResult = new List<ClassDefinition>();
        var structsResult = new List<StructDefinition>();
            
        foreach (var @class in classes)
        {
            if (string.IsNullOrEmpty(@class.SourceFile))
                continue;
            if(!pAllowedClasses.Contains(@class.GetDisplayName()))
                continue;
            
            if(AstUtils.IsIgnoredType(@class))
                continue;
            
            if (@class.TemplateKind != CppTemplateKind.NormalClass)
                throw new NotImplementedException();
            
            // Only process mapped source files. Other modules will be skipped
            var srcFile = Path.GetFullPath(@class.SourceFile);
            if (!allowedSourceFiles.TryGetValue(srcFile, out var moduleItem))
                continue;

            switch (@class.ClassKind)
            {
                case CppClassKind.Class:
                {
                    var classDef = new ClassDefinition(@namespace)
                    {
                        Name = @class.GetDisplayName(),
                        Comment = @class.Comment?.ToString() ?? string.Empty,
                        HeaderFilePath = srcFile,
                        IsAbstract = @class.IsAbstract
                    };

                    classesResult.Add(classDef);
                    pTypes.TryAdd(classDef.GetUniqueName(), (classDef, @class));
                    pCppTypes.TryAdd(@class, classDef);
                }
                    break;
                case CppClassKind.Struct:
                {
                    var structDef = new StructDefinition(@namespace)
                    {
                        Name = @class.GetDisplayName(),
                        Comment = @class.Comment?.ToString() ?? string.Empty,
                        HeaderFilePath = srcFile
                    };
                    
                    structsResult.Add(structDef);
                    pTypes.TryAdd(structDef.GetUniqueName(), (structDef, @class));
                    pCppTypes.TryAdd(@class, structDef);
                }
                    break;
                case CppClassKind.Union:
                    throw new NotImplementedException();
            }
        }
        
        @namespace.Classes = classesResult.ToArray();
        @namespace.Structs = structsResult.ToArray();
    }

    private void CollectClassMethods(NamespaceDefinition @namespace)
    {
        var classes = @namespace.Classes;
        var structs = @namespace.Structs;

        foreach (var @class in classes)
        {
            var (_, classElement) = pTypes[@class.GetUniqueName()];
            var klass = (CppClass)classElement;
            var methods = new List<ClassMethodDefinition>();
            var constructors = new List<ConstructorMethodDefinition>();

            foreach (var func in klass.Functions)
            {
                if(func.IsFunctionTemplate)
                    continue;

                if(func.Visibility != CppVisibility.Public && func.Visibility != CppVisibility.Default)
                    continue;
                
                if (AstUtils.IsIgnoredType(func))
                    continue;
                
                if(AstUtils.IsOperatorMethod(func))
                    continue;

                var returnType = GetSuitableType(func.ReturnType);
                var args = new TypeDefinition[func.Parameters.Count];
                var skip = returnType is null;
                
                for (var i = 0; i < func.Parameters.Count; ++i)
                {
                    var argType = GetSuitableType(func.Parameters[i].Type);
                    if (argType is null)
                    {
                        skip = true;
                        break;
                    }

                    args[i] = argType;
                }

                if (skip)
                {
                    Console.WriteLine($"- Skipping class method {func.Name}. [{func.Span.Start}, {func.Span.End}]");
                    continue;
                }

                var method = new ClassMethodDefinition(@class)
                {
                    Name = func.Name,
                    Comment = func.Comment?.ToString() ?? string.Empty,
                    IsStatic = func.IsStatic,
                    ReturnType = returnType!,
                    ArgumentTypes = args,
                    HeaderFilePath = func.SourceFile,
                };
                methods.Add(method);
            }

            foreach (var ctor in klass.Constructors)
            { 
                if(AstUtils.IsIgnoredType(ctor))
                    continue;
                
                var skip = false;
                var args = new TypeDefinition[ctor.Parameters.Count];
                
                for (var i = 0; i < ctor.Parameters.Count; ++i)
                {
                    var argType = GetSuitableType(ctor.Parameters[i].Type);
                    if (argType is null)
                    {
                        skip = true;
                        break;
                    }

                    args[i] = argType;
                }

                if (skip)
                {
                    Console.WriteLine($"- Skipping class constructor '{ctor}'. [{ctor.Span.Start}, {ctor.Span.End}]");
                    continue;
                }

                ConstructorMethodDefinition ctorDef = new(@class)
                {
                    Name = ctor.Name,
                    Comment = ctor.Comment?.ToString() ?? string.Empty,
                    ArgumentTypes = args,
                    HeaderFilePath = ctor.SourceFile
                };
                constructors.Add(ctorDef);
            }

            @class.Constructors = constructors.ToArray();
            @class.Methods = methods.ToArray();
        }

        foreach (var @struct in structs)
        {
            var (_, structElement) = pTypes[@struct.GetUniqueName()];
            var klass = (CppClass)structElement;
            var methods = new List<StructMethodDefinition>();
            
            foreach (var func in klass.Functions)
            {
                if(func.IsFunctionTemplate)
                    continue;
                
                if(func.Visibility != CppVisibility.Public && func.Visibility != CppVisibility.Default)
                    continue;
                
                if(AstUtils.IsIgnoredType(func))
                    continue;
                
                if(AstUtils.IsOperatorMethod(func))
                    continue;
                
                var returnType = GetSuitableType(func.ReturnType);
                var args = new TypeDefinition[func.Parameters.Count];
                var skip = returnType is null;

                for (var i = 0; i < func.Parameters.Count; ++i)
                {
                    var argType = GetSuitableType(func.Parameters[i].Type);
                    if (argType is null)
                    {
                        skip = true;
                        break;
                    }

                    args[i] = argType;
                }

                var method = new StructMethodDefinition(@struct);
                method.Name = func.Name;
                method.Comment = func.Comment?.ToString() ?? string.Empty;
                method.ReturnType = returnType!;
                method.ArgumentTypes = args;
                method.HeaderFilePath = func.SourceFile;

                methods.Add(method);
            }

            @struct.Methods = methods.ToArray();
        }
        
        foreach(var nextNamespace in @namespace.Namespaces)
            CollectClassMethods(nextNamespace);
    }
    
    private void CollectStaticMethodsFromNamespaces(NamespaceDefinition[] namespaces, IDictionary<string, ModuleItem> allowedSourceFiles)
    {
        foreach (var @namespace in namespaces)
        {
            var (_, type) = pTypes[@namespace.GetUniqueName()];
            var namespaceType = (CppNamespace)type;
            
            CollectStaticMethods(namespaceType.Functions, allowedSourceFiles, @namespace);
            CollectStaticMethodsFromNamespaces(@namespace.Namespaces, allowedSourceFiles);
        }
    }
    private void CollectStaticMethods(CppContainerList<CppFunction> functions,
        IDictionary<string, ModuleItem> allowedSourceFiles, NamespaceDefinition @namespace)
    {
        var methodsResult = new List<StaticMethodDefinition>();
        foreach (var function in functions)
        {
            if (string.IsNullOrEmpty(function.SourceFile) || function.IsFunctionTemplate)
                continue;

            // Only process mapped source files. Other modules will be skipped
            var srcFile = Path.GetFullPath(function.SourceFile);
            if (!allowedSourceFiles.TryGetValue(srcFile, out var moduleItem))
                continue;

            if(AstUtils.IsIgnoredType(function))
                continue;
            
            if(AstUtils.IsOperatorMethod(function))
                continue;
            
            var returnType = GetSuitableType(function.ReturnType);
            var args = new TypeDefinition[function.Parameters.Count];
            
            if(returnType is null)
                continue;

            var skip = false;
            for (var i = 0; i < function.Parameters.Count; ++i)
            {
                var argType = GetSuitableType(function.Parameters[i].Type);
                if (argType is null)
                {
                    skip = true;
                    break;
                }

                args[i] = argType;
            }

            if (skip)
                continue;
            
            var method = new StaticMethodDefinition(@namespace)
            {
                Name = function.Name,
                Comment = function.Comment?.ToString() ?? string.Empty,
                HeaderFilePath = srcFile,
                ReturnType = returnType,
                ArgumentTypes = args
            };

            methodsResult.Add(method);
        }

        @namespace.Methods = methodsResult.ToArray();
    }

    private TypeDefinition? GetType(CppType type)
    {
        if(!pCppTypes.TryGetValue(type, out var result))
            Console.WriteLine($"- Not found type of {type.FullName}.\n [{type.Span.Start}, {type.Span.End}]");
        return result;
    }
    private TypeDefinition? GetSuitableType(CppType type, CppType? prevType = null)
    {
        if (PrimitiveTypeDefinition.IsString(type))
            return new PrimitiveTypeDefinition(PrimitiveKind.String);
        
        if(PrimitiveTypeDefinition.IsStringHash(type))
           return new PrimitiveTypeDefinition(PrimitiveKind.StringHash);

        if (PrimitiveTypeDefinition.IsVariant(type))
            return new PrimitiveTypeDefinition(PrimitiveKind.Variant);
        
        TypeDefinition? result = null;
        switch (type.TypeKind)
        {
            case CppTypeKind.Primitive:
                result = new PrimitiveTypeDefinition((PrimitiveKind)((CppPrimitiveType)type).Kind);
                break;
            case CppTypeKind.Pointer:
            {
                var nextType = GetSuitableType(((CppPointerType)type).ElementType, type);
                if (nextType != null)
                    result = new PointerTypeDefinition(nextType);
            }
                break;
            case CppTypeKind.Reference:
            {
                var nextType = GetSuitableType(((CppReferenceType)type).ElementType, type);
                if (nextType != null)
                    result = new ReferenceTypeDefinition(nextType);
            }
                break;
            case CppTypeKind.StructOrClass:
            {
                var klass = (CppClass)type;
                if (VectorDefinition.IsVector(type))
                {
                    var targetType = GetSuitableType(klass.TemplateSpecializedArguments[0].ArgAsType, type);
                    if (targetType is not null)
                        result = new VectorDefinition(targetType, VectorDefinition.GetVectorType(type));
                } 
                else if (HashMapDefinition.IsHashMap(type))
                {
                    var targetType = GetSuitableType(klass.TemplateSpecializedArguments[0].ArgAsType, type);
                    if (targetType is not null)
                        result = new HashMapDefinition(targetType);
                }
                else if (SmartPointerTypeDefinition.IsSmartPointer(type))
                {
                    var targetType = GetSuitableType(klass.TemplateSpecializedArguments[0].ArgAsType, type);
                    if (targetType is not null)
                    {
                        var smartPtr = new SmartPointerTypeDefinition(targetType);
                        smartPtr.IsWeak = SmartPointerTypeDefinition.IsWeakPtr(type);
                        result = smartPtr;
                    }
                }
                else
                    result = GetType(type);
            }
                break;
            case CppTypeKind.Enum:
                result = GetType(type);
                break;
            case CppTypeKind.Qualified:
            {
                var qualifiedType = (CppQualifiedType)type;   
                if (string.Equals(type.FullName, "char const"))
                    result = new PrimitiveTypeDefinition(PrimitiveKind.String);
                else if (qualifiedType.Qualifier == CppTypeQualifier.Const)
                    result = GetSuitableType(qualifiedType.ElementType, prevType);
                else
                    Console.WriteLine($"- Unsupported Qualified Type '{qualifiedType.FullName}'.\n [{qualifiedType.Span.Start}, {qualifiedType.Span.End}]");
            }
                break;
            case CppTypeKind.Typedef:
            {
                var typeDef = (CppTypedef)type;
                // If previous type is a pointer, then we must deal as void*
                if (prevType?.TypeKind == CppTypeKind.Pointer)
                    result = new PrimitiveTypeDefinition(PrimitiveKind.Void);
                else if (string.Equals(type.GetDisplayName(), "VariantMap"))
                    result = new PrimitiveTypeDefinition(PrimitiveKind.VariantMap);
                else
                    result = GetSuitableType(typeDef.ElementType, prevType);
            }
                break;
            case CppTypeKind.Unexposed:
                Console.WriteLine($"- Unsupported Unexposed Type '{type.FullName}'.\n [{type.Span.Start}, {type.Span.End}]");
                break;
            default:
                throw new NotImplementedException();
        }

        return result;
    }
}