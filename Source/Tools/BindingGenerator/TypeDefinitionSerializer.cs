using System.Text.Json;
using System.Text.Json.Serialization;
using BindingGenerator.Models;

namespace BindingGenerator;

public class TypeDefinitionSerializer(NamespaceDefinition rootNamespace)
{
    private List<TypeDefSerializeData> pTypes = new();
    private Dictionary<TypeDefinition, TypeDefSerializeData> pLookupTbl = new();
    
    public void Build()
    {
        pTypes.Clear();
        pLookupTbl.Clear();

        var rootNamespaceTypeDef = new TypeDefSerializeData()
        {
            Id = pTypes.Count,
            Name = rootNamespace.Name,
            Comment = rootNamespace.Comment,
            HeaderFilePath = rootNamespace.HeaderFilePath,
            Kind = TypeDefKind.Namespace,
            NamespaceData = new NamespaceSerializeData()
        };
        pTypes.Add(rootNamespaceTypeDef);
        pLookupTbl[rootNamespace] = rootNamespaceTypeDef;
        
        CollectNamespaces(rootNamespaceTypeDef, rootNamespace.Namespaces);
        
        var namespaces = new[] { rootNamespace };
        CollectEnums(namespaces);
        CollectClasses(namespaces);
        CollectStructs(namespaces);
        CollectStaticMethods(namespaces);
        
        CollectClassMethods(namespaces);
        CollectStructMethods(namespaces);
        
        CollectStaticMethodTypes(namespaces);
        CollectClassMethodTypes(namespaces);
        CollectClassFieldTypes(namespaces);
        CollectStructMethodTypes(namespaces);
        CollectStructFieldTypes(namespaces);
    }

    private void CollectNamespaces(TypeDefSerializeData parentNamespace, NamespaceDefinition[] namespaces)
    {
        if (parentNamespace.NamespaceData is null)
            throw new NullReferenceException();
        
        var nsIds = new List<int>();
        foreach (var ns in namespaces)
        {
            var type = new TypeDefSerializeData()
            {
                Id = pTypes.Count,
                Name = ns.Name,
                Comment = ns.Comment,
                HeaderFilePath = ns.HeaderFilePath,
                Kind = TypeDefKind.Namespace,
                NamespaceData = new NamespaceSerializeData()
            };
            
            pTypes.Add(type);
            nsIds.Add(type.Id);
            pLookupTbl[ns] = type;
        }


        parentNamespace.NamespaceData.Namespaces = nsIds.ToArray();
        // Repeat again loop but collect nested namespaces
        foreach (var ns in namespaces)
            CollectNamespaces(pLookupTbl[ns], ns.Namespaces);
    }

    private void CollectEnums(NamespaceDefinition[] namespaceDefs)
    {
        foreach (var ns in namespaceDefs)
        {
            var enumIds = new List<int>();
            var nsType = pLookupTbl[ns];
            
            foreach (var @enum in ns.Enums)
            {
                var type = new TypeDefSerializeData()
                {
                    Id = pTypes.Count(),
                    Name = @enum.Name,
                    Comment = @enum.Comment,
                    HeaderFilePath = @enum.HeaderFilePath,
                    Kind = TypeDefKind.Enum,
                    EnumData = new EnumSerializeData()
                    {
                        Namespace = pLookupTbl[ns].Id,
                        Entries = @enum.Entries.Select(x => new EnumSerializeData.Entry()
                        {
                            Comment = x.Comment,
                            Name = x.Name,
                            Value = x.Value
                        }).ToArray()
                    }
                };
                
                pTypes.Add(type);
                enumIds.Add(type.Id);
                pLookupTbl[@enum] = type;
            }

            if (nsType.NamespaceData is not null)
                nsType.NamespaceData.Enums = enumIds.ToArray();
        }

        foreach (var ns in namespaceDefs)
            CollectEnums(ns.Namespaces);
    }

    private void CollectClasses(NamespaceDefinition[] namespaceDefs)
    {
        foreach (var ns in namespaceDefs)
        {
            var namespaceType = pLookupTbl[ns];
            var classIds = new List<int>();
            
            foreach (var klass in ns.Classes)
            {
                var type = new TypeDefSerializeData()
                {
                    Id = pTypes.Count,
                    Name = klass.Name,
                    Comment = klass.Comment,
                    HeaderFilePath = klass.HeaderFilePath,
                    Kind = TypeDefKind.Class,
                    ClassData = new ClassSerializeData()
                    {
                        Namespace = namespaceType.Id,
                        IsAbstract = klass.IsAbstract
                    }
                };
                
                pTypes.Add(type);
                classIds.Add(type.Id);
                
                pLookupTbl[klass] = type;
            }

            if(namespaceType.NamespaceData is not null)
                namespaceType.NamespaceData.Classes = classIds.ToArray();
        }
        
        foreach (var ns in namespaceDefs)
            CollectClasses(ns.Namespaces);
    }

    private void CollectStructs(NamespaceDefinition[] namespaceDefs)
    {
        foreach (var ns in namespaceDefs)
        {
            var namespaceType = pLookupTbl[ns];
            var structIds = new List<int>();
            foreach (var @struct in ns.Structs)
            {
                var type = new TypeDefSerializeData()
                {
                    Id = pTypes.Count,
                    Name = @struct.Name,
                    Comment = @struct.Comment,
                    HeaderFilePath = @struct.HeaderFilePath,
                    Kind = TypeDefKind.Struct,
                    NamespaceData = new NamespaceSerializeData(),
                    StructData = new StructSerializeData() { Namespace = namespaceType.Id }
                };
                pTypes.Add(type);
                structIds.Add(type.Id);
                
                pLookupTbl[@struct] = type;
            }
            
            if(namespaceType.NamespaceData is not null)
                namespaceType.NamespaceData.Structs = structIds.ToArray();
        }
        
        foreach(var ns in namespaceDefs)
            CollectStructs(ns.Namespaces);
    }
    
    private void CollectStaticMethods(NamespaceDefinition[] namespaceDefs)
    {
        foreach (var ns in namespaceDefs)
        {
            var nsType = pLookupTbl[ns];
            var methodIds = new List<int>();
            
            foreach (var method in ns.Methods)
            {
                var type = new TypeDefSerializeData()
                {
                    Id = pTypes.Count,
                    Name = ns.Name,
                    Comment = ns.Comment,
                    HeaderFilePath = ns.HeaderFilePath,
                    Kind = TypeDefKind.StaticMethod,
                    MethodData = new MethodSerializeData()
                    {
                        Owner = pLookupTbl[ns].Id,
                        IsStatic = true
                    }
                };
                
                pTypes.Add(type);
                methodIds.Add(type.Id);
                pLookupTbl[method] = type;
            }

            if (nsType.NamespaceData is not null)
                nsType.NamespaceData.Methods = methodIds.ToArray();
        }

        foreach (var ns in namespaceDefs)
            CollectStaticMethods(ns.Namespaces);
    }

    private void CollectClassMethods(NamespaceDefinition[] namespaceDefs)
    {
        foreach (var ns in namespaceDefs)
        {
            foreach (var klass in ns.Classes)
            {
                var classType = pLookupTbl[klass];
                var methodIds = new List<int>();
                var fieldIds = new List<int>();
                var currentCtors = new Dictionary<string, TypeDefSerializeData>();
                
                foreach (var method in klass.Methods)
                {
                    var type = new TypeDefSerializeData()
                    {
                        Id = pTypes.Count,
                        Name = method.Name,
                        Comment = method.Comment,
                        HeaderFilePath = method.HeaderFilePath,
                        Kind = TypeDefKind.ClassMethod,
                        MethodData = new MethodSerializeData()
                        {
                            IsStatic = method.IsStatic,
                            Owner = classType.Id
                        }
                    };
                    
                    pTypes.Add(type);
                    pLookupTbl[method] = type;
                    methodIds.Add(type.Id);
                }
                
                foreach (var ctor in klass.Constructors)
                {
                    var type = new TypeDefSerializeData()
                    {
                        Id = pTypes.Count,
                        Name = ctor.Name,
                        Comment = ctor.Comment,
                        HeaderFilePath = ctor.HeaderFilePath,
                        Kind = TypeDefKind.Constructor,
                        MethodData = new MethodSerializeData()
                        {
                            Owner = classType.Id
                        }
                    };

                    var ctorKey = ctor.GetUniqueName();
                    // prevent duplicated constructors 
                    if(!currentCtors.TryAdd(ctorKey, type))
                        continue;
                    
                    pTypes.Add(type);
                    pLookupTbl[ctor] = type;
                }

                foreach (var field in klass.Fields)
                {
                    var type = new TypeDefSerializeData()
                    {
                        Id = pTypes.Count,
                        Name = field.Name,
                        Comment = field.Comment,
                        HeaderFilePath = field.HeaderFilePath,
                        Kind = TypeDefKind.Field,
                        FieldData = new FieldSerializeData() { Owner = classType.Id }
                    };
                    
                    pTypes.Add(type);
                    pLookupTbl[field] = type;
                    fieldIds.Add(type.Id);
                }

                if (classType.ClassData is null)
                    throw new NullReferenceException();
                classType.ClassData.Methods = methodIds.ToArray();
                classType.ClassData.Fields = fieldIds.ToArray();
                classType.ClassData.Constructors = currentCtors.Select(x => x.Value.Id).ToArray();
            }
        }

        foreach (var ns in namespaceDefs)
            CollectClassMethods(ns.Namespaces);
    }

    private void CollectStructMethods(NamespaceDefinition[] namespaceDefs)
    {
        foreach (var ns in namespaceDefs)
        {
            foreach (var @struct in ns.Structs)
            {
                var structType = pLookupTbl[@struct];
                var methodIds = new List<int>();
                var fieldIds = new List<int>();
                foreach (var method in @struct.Methods)
                {
                    var type = new TypeDefSerializeData()
                    {
                        Id = pTypes.Count,
                        Name = method.Name,
                        Comment = method.Comment,
                        HeaderFilePath = method.HeaderFilePath,
                        Kind = TypeDefKind.StructMethod,
                        MethodData = new MethodSerializeData() { Owner = structType.Id }
                    };
                    
                    pTypes.Add(type);
                    pLookupTbl[method] = type;
                    methodIds.Add(type.Id);
                }

                foreach (var field in @struct.Fields)
                {
                    var type = new TypeDefSerializeData()
                    {
                        Id = pTypes.Count,
                        Name = field.Name,
                        Comment = field.Comment,
                        HeaderFilePath = field.HeaderFilePath,
                        Kind = TypeDefKind.Field,
                        FieldData = new FieldSerializeData()
                        {
                            Owner = structType.Id
                        }
                    };
                    
                    pTypes.Add(type);
                    pLookupTbl[field] = type;
                    fieldIds.Add(type.Id);
                }
                
                if (structType.StructData is null)
                    throw new NullReferenceException();
                
                structType.StructData.Methods = methodIds.ToArray();
                structType.StructData.Fields = fieldIds.ToArray();
            }
        }

        foreach (var ns in namespaceDefs)
            CollectStructMethods(ns.Namespaces);
    }
    /**
     * After all types registered.
     * We must collect all function return and argument types and resolve their id's
     */
    private void CollectStaticMethodTypes(NamespaceDefinition[] namespaceDefs)
    {
        foreach (var ns in namespaceDefs)
        {
            foreach (var method in ns.Methods)
            {
                var methodDef = pLookupTbl[method];
                if (methodDef.MethodData is null)
                    throw new NullReferenceException();

                methodDef.MethodData.ReturnType =
                    GetOrRegisterType(method.ReturnType, CreateTypeDef(method.ReturnType));
                methodDef.MethodData.ArgTypes = method.ArgumentTypes
                    .Select(x => GetOrRegisterType(x, CreateTypeDef(x)))
                    .ToArray();
            }
        }

        foreach (var ns in namespaceDefs)
            CollectStaticMethodTypes(ns.Namespaces);
    }

    private void CollectClassMethodTypes(NamespaceDefinition[] namespaceDefs)
    {
        foreach (var ns in namespaceDefs)
        {
            foreach (var klass in ns.Classes)
            {
                foreach (var method in klass.Methods)
                {
                    var methodDef = pLookupTbl[method];
                    if (methodDef.MethodData is null)
                        throw new NullReferenceException();

                    methodDef.MethodData.ReturnType =
                        GetOrRegisterType(method.ReturnType, CreateTypeDef(method.ReturnType));
                    methodDef.MethodData.ArgTypes = method.ArgumentTypes
                        .Select(x => GetOrRegisterType(x, CreateTypeDef(x)))
                        .ToArray();
                }

                foreach (var ctor in klass.Constructors)
                {
                    var ctorDef = pLookupTbl[ctor];
                    if (ctorDef.MethodData is null)
                        throw new NullReferenceException();

                    ctorDef.MethodData.ArgTypes = ctor.ArgumentTypes
                        .Select(x => GetOrRegisterType(x, CreateTypeDef(x)))
                        .ToArray();
                }
            }
        }

        foreach (var ns in namespaceDefs)
            CollectClassMethodTypes(ns.Namespaces);
    }

    private void CollectClassFieldTypes(NamespaceDefinition[] namespaceDefs)
    {
        foreach (var ns in namespaceDefs)
        {
            foreach (var klass in ns.Classes)
            {
                foreach (var field in klass.Fields)
                {
                    var fieldDef = pLookupTbl[field];
                    if (fieldDef.FieldData is null)
                        throw new NullReferenceException();
                    fieldDef.FieldData.Type = GetOrRegisterType(field.Type, CreateTypeDef(field.Type));
                }
            }
        }

        foreach (var ns in namespaceDefs)
            CollectClassFieldTypes(ns.Namespaces);
    }
    private void CollectStructMethodTypes(NamespaceDefinition[] namespaceDefs)
    {
        foreach (var ns in namespaceDefs)
        {
            foreach (var @struct in ns.Structs)
            {
                foreach (var method in @struct.Methods)
                {
                    var methodDef = pLookupTbl[method];
                    if (methodDef.MethodData is null)
                        throw new NullReferenceException();

                    methodDef.MethodData.ReturnType =
                        GetOrRegisterType(method.ReturnType, CreateTypeDef(method.ReturnType));
                    methodDef.MethodData.ArgTypes = method.ArgumentTypes
                        .Select(x => GetOrRegisterType(x, CreateTypeDef(x)))
                        .ToArray();
                }
            }
        }

        foreach (var ns in namespaceDefs)
            CollectStructMethodTypes(ns.Namespaces);
    }

    private void CollectStructFieldTypes(NamespaceDefinition[] namespaceDefs)
    {
        foreach (var ns in namespaceDefs)
        {
            foreach (var @struct in ns.Structs)
            {
                foreach (var field in @struct.Fields)
                {
                    var fieldDef = pLookupTbl[field];
                    if (fieldDef.FieldData is null)
                        throw new NullReferenceException();
                    fieldDef.FieldData.Type = GetOrRegisterType(field.Type, CreateTypeDef(field.Type));
                }
            }
        }

        foreach (var ns in namespaceDefs)
            CollectStructFieldTypes(ns.Namespaces);
    }
    
    private TypeDefSerializeData CreateTypeDef(TypeDefinition type)
    {
        var typeDef = new TypeDefSerializeData()
        {
            Id = pTypes.Count(),
            Comment = type.Comment,
            HeaderFilePath = type.HeaderFilePath,
            Kind = type.Kind
        };
        var currTypeDef = typeDef;

        switch (type.Kind)
        {
            case TypeDefKind.Vector:
            {
                var vecType = (VectorDefinition)type;
                typeDef.VectorData = new VectorData()
                {
                    Type = vecType.Type,
                    ElementType = CreateTypeDef(vecType.ElementType).Id,
                };
            }
                break;
            case TypeDefKind.Pointer:
                typeDef.TypeData = CreateTypeDef(((PointerTypeDefinition)type).Type).Id;
                break;
            case TypeDefKind.SmartPtr:
                typeDef.SmartPointerData = new SmartPointerData()
                {
                    Type = CreateTypeDef(((SmartPointerTypeDefinition)type).Type).Id,
                    IsWeak = ((SmartPointerTypeDefinition)type).IsWeak
                };
                break;
            case TypeDefKind.Ref:
                typeDef.TypeData = CreateTypeDef(((ReferenceTypeDefinition)type).Type).Id;
                break;
            case TypeDefKind.Primitive:
                typeDef.PrimitiveData = ((PrimitiveTypeDefinition)type).PrimitiveKind;
                break;
            case TypeDefKind.HashMap:
                typeDef.TypeData = CreateTypeDef(((HashMapDefinition)type).Type).Id;
                break;
            case TypeDefKind.Unknow:
            case TypeDefKind.Namespace:
            case TypeDefKind.Enum:
            case TypeDefKind.Class:
            case TypeDefKind.Struct:
            case TypeDefKind.StaticMethod:
            case TypeDefKind.ClassMethod:
            case TypeDefKind.Constructor:
            case TypeDefKind.StructMethod:
            default:
                typeDef = pLookupTbl[type];
                break;
        }

        if(currTypeDef == typeDef)
            pTypes.Add(typeDef);
        return typeDef;
    }

    private int GetOrRegisterType(TypeDefinition type, TypeDefSerializeData type2Insert)
    {
        if (pLookupTbl.TryGetValue(type, out var res))
            return res.Id;
        pLookupTbl[type] = type2Insert;
        type2Insert.Id = pTypes.Count;
        pTypes.Add(type2Insert);
        return type2Insert.Id;
    }
    
    public void Serialize(Stream stream)
    {
        using TextWriter writer = new StreamWriter(stream);
        writer.Write(JsonSerializer.Serialize(pTypes, new JsonSerializerOptions() { WriteIndented = true }));
    }
}