namespace BindingGenerator.Models;

public class NamespaceSerializeData
{
    public int Owner { get; set; }
    public int[] Enums { get; set; } = [];
    public int[] Classes { get; set; } = [];
    public int[] Structs { get; set; } = [];
    public int[] Methods { get; set; } = [];
    public int[] Namespaces { get; set; } = [];
}

public class EnumSerializeData
{
    public class Entry
    {
        public string Comment { get; set; } = string.Empty;
        public string Name { get; set; } = string.Empty;
        public int Value { get; set; }
    }

    public Entry[] Entries { get; set; } = [];
    public int Namespace { get; set; }
}

public class ClassSerializeData
{
    public bool IsAbstract { get; set; } = false;
    public int Namespace { get; set; }
    public int[] Methods { get; set; } = [];
    public int[] Constructors { get; set; } = [];
}

public class StructSerializeData
{
    public int Namespace { get; set; } = -1;
    public int[] Methods { get; set; } = [];
}

public class MethodSerializeData
{
    public bool IsStatic { get; set; }
    public int Owner { get; set; }
    public int ReturnType { get; set; }
    public int[] ArgTypes { get; set; } = [];
}

public class SmartPointerData
{
    public int Type { get; set; } = -1;
    public bool IsWeak { get; set; }
}

public class VectorData
{
    public VectorType Type { get; set; }
    public int ElementType { get; set; } = -1;
}
public class TypeDefSerializeData
{
    // Id must be the same of type index at types list.
    public int Id { get; set; }
    public TypeDefKind Kind { get; set; }
    public string Name { get; set; } = string.Empty;
    public string Comment { get; set; } = string.Empty;
    public string HeaderFilePath { get; set; } = string.Empty;
    
    public NamespaceSerializeData? NamespaceData { get; set; }
    public EnumSerializeData? EnumData { get; set; }
    public ClassSerializeData? ClassData { get; set; }
    public StructSerializeData? StructData { get; set; }
    public MethodSerializeData? MethodData { get; set; }
    public PrimitiveKind PrimitiveData { get; set; } = PrimitiveKind.Unknow;
    public int TypeData { get; set; } = -1;
    public SmartPointerData? SmartPointerData { get; set; }
    public VectorData? VectorData { get; set; }
}