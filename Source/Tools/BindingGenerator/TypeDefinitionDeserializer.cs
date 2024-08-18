using System.Text.Json;
using System.Text.Json.Serialization;
using BindingGenerator.Models;

namespace BindingGenerator;

public class TypeDefinitionDeserializer
{
    private TypeDefSerializeData[] pTypes = [];
    private NamespaceDefinition? pRootNamespace;

    private Dictionary<TypeDefSerializeData, TypeDefinition> pTypesMap = new();

    public NamespaceDefinition Namespace => pRootNamespace ?? throw new NullReferenceException();
    public int TotalOfTypes => pTypes.Length;

    public void Deserialize(Stream stream)
    {
        pRootNamespace = null;
        pTypesMap.Clear();
        
        using var reader = new StreamReader(stream);
        pTypes = JsonSerializer.Deserialize<TypeDefSerializeData[]>(reader.ReadToEnd()) ??
                 throw new NullReferenceException();

        // if namespace root doesn't exist. Find first available namespace.
        // this is not recommend and must be revisited.
        var rootNamespaceDef = pTypes.FirstOrDefault(x => x.Id == 0) ?? pTypes.First(x => x.Kind == TypeDefKind.Namespace);
        pRootNamespace = BuildNamespace(rootNamespaceDef);
    }

    private NamespaceDefinition BuildNamespace(TypeDefSerializeData namespaceDef)
    {
        if (namespaceDef.Kind != TypeDefKind.Namespace || namespaceDef.NamespaceData is null)
            throw new ArgumentException("Invalid Namespace Type Definition");

        if (pTypesMap.TryGetValue(namespaceDef, out var typeRes))
            return (NamespaceDefinition)typeRes;

        var ownerId = namespaceDef.NamespaceData.Owner;
        var ns = namespaceDef.Id == 0 ? new GlobalNamespaceDefinition() : new NamespaceDefinition();
        FillTypeInfo(ns, namespaceDef);
        pTypesMap[namespaceDef] = ns;
        
        if (ownerId != namespaceDef.Id)
            ns.Owner = BuildNamespace(pTypes[ownerId]);

        ns.Namespaces = namespaceDef.NamespaceData.Namespaces
            .Select(x => BuildNamespace(pTypes[x])).ToArray();
        ns.Enums = namespaceDef.NamespaceData.Enums
            .Select(x => BuildEnum(pTypes[x])).ToArray();
        ns.Classes = namespaceDef.NamespaceData.Classes
            .Select(x => BuildClass(pTypes[x])).ToArray();
        ns.Structs = namespaceDef.NamespaceData.Structs
            .Select(x => BuildStruct(pTypes[x])).ToArray();
        ns.Methods = namespaceDef.NamespaceData.Methods
            .Select(x => BuildStaticMethod(pTypes[x])).ToArray();
        return ns;
    }

    private EnumDefinition BuildEnum(TypeDefSerializeData enumDef)
    {
        if (enumDef.Kind != TypeDefKind.Enum || enumDef.EnumData is null)
            throw new ArgumentException("Invalid Enum Type Definition");

        if (pTypesMap.TryGetValue(enumDef, out var typeRes))
            return (EnumDefinition)typeRes;
        
        var ns = BuildNamespace(pTypes[enumDef.EnumData.Namespace]);
        var e = new EnumDefinition(ns);
        FillTypeInfo(e, enumDef);
        pTypesMap[enumDef] = e;

        e.Entries = enumDef.EnumData.Entries.Select(x => new EnumEntry(e)
        {
            Name = x.Name,
            Comment = x.Comment,
            Value = x.Value
        }).ToArray();

        return e;
    }
    private ClassDefinition BuildClass(TypeDefSerializeData classDef)
    {
        if (classDef.Kind != TypeDefKind.Class || classDef.ClassData is null)
            throw new ArgumentException("Invalid Class Type Definition");

        if (pTypesMap.TryGetValue(classDef, out var typeRes))
            return (ClassDefinition)typeRes;

        var ns = BuildNamespace(pTypes[classDef.ClassData.Namespace]);
        
        var klass = new ClassDefinition(ns);
        FillTypeInfo(klass, classDef);
        pTypesMap[classDef] = klass;
        
        klass.IsAbstract = classDef.ClassData.IsAbstract;
        klass.Constructors = classDef.ClassData.Constructors
            .Select(ctorId => BuildConstructor(pTypes[ctorId]))
            .ToArray();
        klass.Methods = classDef.ClassData.Methods
            .Select(methodId => BuildClassMethod(pTypes[methodId]))
            .ToArray();
        
        return klass;
    }
    private StructDefinition BuildStruct(TypeDefSerializeData structDef)
    {
        if (structDef.Kind != TypeDefKind.Struct || structDef.StructData is null)
            throw new ArgumentException("Invalid Struct Type Definition");

        if (pTypesMap.TryGetValue(structDef, out var typeRes))
            return (StructDefinition)typeRes;

        var ns = BuildNamespace(pTypes[structDef.StructData.Namespace]);

        var @struct = new StructDefinition(ns);
        FillTypeInfo(@struct, structDef);

        pTypesMap[structDef] = @struct;

        @struct.Methods = structDef.StructData.Methods
            .Select(x => BuildStructMethod(pTypes[x]))
            .ToArray();
        return @struct;
    }
    private ConstructorMethodDefinition BuildConstructor(TypeDefSerializeData ctorDef)
    {
        if (ctorDef.Kind != TypeDefKind.Constructor || ctorDef.MethodData is null)
            throw new ArgumentException("Invalid Constructor Type Definition");

        if (pTypesMap.TryGetValue(ctorDef, out var typeRes))
            return (ConstructorMethodDefinition)typeRes;

        var klass = BuildClass(pTypes[ctorDef.MethodData.Owner]);
        var ctor = new ConstructorMethodDefinition(klass);
        FillTypeInfo(ctor, ctorDef);
        pTypesMap[ctorDef] = ctor;

        ctor.ArgumentTypes = ctorDef.MethodData.ArgTypes.Select(x => GetType(pTypes[x])).ToArray();
        return ctor;
    }
    private ClassMethodDefinition BuildClassMethod(TypeDefSerializeData methodDef)
    {
        if (methodDef.Kind != TypeDefKind.ClassMethod || methodDef.MethodData is null)
            throw new ArgumentException("Invalid Class Method Type Definition");

        if (pTypesMap.TryGetValue(methodDef, out var typeRes))
            return (ClassMethodDefinition)typeRes;

        var klass = BuildClass(pTypes[methodDef.MethodData.Owner]);
        var method = new ClassMethodDefinition(klass);
        FillTypeInfo(method, methodDef);
        pTypesMap[methodDef] = method;

        method.IsStatic = methodDef.MethodData.IsStatic;
        method.ReturnType = GetType(pTypes[methodDef.MethodData.ReturnType]);
        method.ArgumentTypes = methodDef.MethodData.ArgTypes
            .Select(x => GetType(pTypes[x]))
            .ToArray();
        return method;
    }
    private StructMethodDefinition BuildStructMethod(TypeDefSerializeData methodDef)
    {
        if (methodDef.Kind != TypeDefKind.StructMethod || methodDef.MethodData is null)
            throw new ArgumentException("Invalid Struct Method Type Definition");
        
        if (pTypesMap.TryGetValue(methodDef, out var typeRes))
            return (StructMethodDefinition)typeRes;

        var @struct = BuildStruct(pTypes[methodDef.MethodData.Owner]);
        var method = new StructMethodDefinition(@struct);

        pTypesMap[methodDef] = method;

        method.ReturnType = GetType(pTypes[methodDef.MethodData.ReturnType]);
        method.ArgumentTypes = methodDef.MethodData.ArgTypes
            .Select(x => GetType(pTypes[x]))
            .ToArray();
        return method;
    }
    private StaticMethodDefinition BuildStaticMethod(TypeDefSerializeData methodDef)
    {
        if (methodDef.Kind != TypeDefKind.StaticMethod || methodDef.MethodData is null)
            throw new ArgumentException("Invalid Struct Method Type Definition");
        
        if (pTypesMap.TryGetValue(methodDef, out var typeRes))
            return (StaticMethodDefinition)typeRes;

        var ns = BuildNamespace(pTypes[methodDef.MethodData.Owner]);
        var method = new StaticMethodDefinition(ns);
        FillTypeInfo(method, methodDef);
        pTypesMap[methodDef] = method;
        
        method.ReturnType = GetType(pTypes[methodDef.MethodData.ReturnType]);
        method.ArgumentTypes = methodDef.MethodData.ArgTypes
            .Select(x => GetType(pTypes[x]))
            .ToArray();

        return method;
    }
    private TypeDefinition GetType(TypeDefSerializeData typeDef)
    {
        TypeDefinition result = UnknowTypeDefinition.Default;
        switch (typeDef.Kind)
        {
            case TypeDefKind.Class:
                result = BuildClass(typeDef);
                break;
            case TypeDefKind.Namespace:
                result = BuildNamespace(typeDef);
                break;
            case TypeDefKind.Enum:
                result = BuildEnum(typeDef);
                break;
            case TypeDefKind.Struct:
                result = BuildStruct(typeDef);
                break;
            case TypeDefKind.StaticMethod:
                break;
            case TypeDefKind.ClassMethod:
                result = BuildClassMethod(typeDef);
                break;
            case TypeDefKind.Constructor:
                result = BuildConstructor(typeDef);
                break;
            case TypeDefKind.StructMethod:
                result = BuildStructMethod(typeDef);
                break;
            case TypeDefKind.Vector:
            {
                if (typeDef.VectorData is null)
                    throw new NullReferenceException();
                result = new VectorDefinition(GetType(pTypes[typeDef.VectorData.ElementType]), typeDef.VectorData.Type);
            }
                break;
            case TypeDefKind.SmartPtr:
            {
                if (typeDef.SmartPointerData is null)
                    throw new NullReferenceException();
                result = new SmartPointerTypeDefinition(
                    GetType(pTypes[typeDef.SmartPointerData.Type]),
                    typeDef.SmartPointerData.IsWeak
                );
            }
                break;
            case TypeDefKind.Ref:
                result = new ReferenceTypeDefinition(GetType(pTypes[typeDef.TypeData]));
                break;
            case TypeDefKind.Primitive:
                result = new PrimitiveTypeDefinition(typeDef.PrimitiveData);
                break;
            case TypeDefKind.Pointer:
                result = new PointerTypeDefinition(GetType(pTypes[typeDef.TypeData]));
                break;
            case TypeDefKind.HashMap:
                result = new HashMapDefinition(GetType(pTypes[typeDef.TypeData]));
                break;
            case TypeDefKind.Unknow:
            default:
                throw new ArgumentOutOfRangeException();
        }

        return result;
    }
    
    private void FillTypeInfo(TypeDefinition target, TypeDefSerializeData source)
    {
        target.Name = source.Name;
        target.Comment = source.Comment;
        target.HeaderFilePath = source.HeaderFilePath;
    }
}