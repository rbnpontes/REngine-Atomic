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
    
    public NamespaceDefinition Namespace { get; private set; } = new GlobalNamespaceDefinition(createDesc.Module, null);
    public int TotalOfTypes => pTypes.Count;

    public void Collect()
    {
        pTypes.Clear();
        pCppTypes.Clear();
        pAllowedClasses.Clear();
        
        Namespace = new GlobalNamespaceDefinition(createDesc.Module, null);

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
            var namespaceDef = new NamespaceDefinition(createDesc.Module, null);
            namespaceDef.Owner = @namespace;
            namespaceDef.Name = @namespace.Name;
            namespaceDef.Comment = @namespace.Comment?.ToString() ?? string.Empty;

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
            
            var enumDef = new EnumDefinition(createDesc.Module, moduleItem, @namespace)
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
                    var classDef = new ClassDefinition(createDesc.Module, moduleItem, @namespace)
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
                    var structDef = new StructDefinition(createDesc.Module, moduleItem, @namespace)
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

            foreach (var func in klass.Functions)
            {
                if(func.IsFunctionTemplate)
                    continue;
            }
        }

        foreach (var @struct in structs)
        {
            
        }
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
            
            var method = new StaticMethodDefinition(createDesc.Module, moduleItem, @namespace)
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

    private TypeDefinition? GetSuitableType(CppType type, CppType? prevType = null)
    {
        TypeDefinition? result = null;
        switch (type.TypeKind)
        {
            case CppTypeKind.Primitive:
                result = new PrimitiveTypeDefinition(createDesc.Module, null, (PrimitiveKind)((CppPrimitiveType)type).Kind);
                break;
            case CppTypeKind.Pointer:
            {
                var nextType = GetSuitableType(((CppPointerType)type).ElementType, type);
                if (nextType != null)
                {
                    result = new PointerTypeDefinition(
                        createDesc.Module, 
                        null, 
                        new PointerTypeDefinition(
                            createDesc.Module, 
                            null, 
                            nextType
                        )
                    );
                }
            }
                break;
            case CppTypeKind.Reference:
            {
                var nextType = GetSuitableType(((CppReferenceType)type).ElementType, type);
                if (nextType != null)
                {
                    result = new ReferenceTypeDefinition(
                        createDesc.Module,
                        null,
                        new PointerTypeDefinition(
                            createDesc.Module,
                            null,
                            nextType)
                    );
                }
            }
                break;
            case CppTypeKind.StructOrClass:
            case CppTypeKind.Enum:
                pCppTypes.TryGetValue(type, out result);
                break;
            case CppTypeKind.Qualified:
            {
                var qualifiedType = (CppQualifiedType)type;   
                if (string.Equals(type.FullName, "char const"))
                    result = new PrimitiveTypeDefinition(createDesc.Module, null, PrimitiveKind.String);
                else if (qualifiedType.Qualifier == CppTypeQualifier.Const)
                    result = GetSuitableType(qualifiedType.ElementType, prevType);
                else
                    throw new NotImplementedException();
            }
                break;
            case CppTypeKind.Typedef:
            {
                // If previous type is a pointer, then we must deal as void*
                if (prevType?.TypeKind == CppTypeKind.Pointer)
                    result = new PrimitiveTypeDefinition(createDesc.Module, null, PrimitiveKind.Void);
                else if (string.Equals(type.GetDisplayName(), "VariantMap"))
                    result = new PrimitiveTypeDefinition(createDesc.Module, null, PrimitiveKind.VariantMap);
                else
                    Console.WriteLine($"- Not found suitable type for: {type.FullName}");
            }
                break;
            default:
                throw new NotImplementedException();
        }

        return result;
    }
}