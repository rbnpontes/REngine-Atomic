namespace BindingGenerator.Models;

public enum TypeDefKind
{
    Unknow = 0,
    Namespace,
    Enum,
    Class,
    Struct,
    StaticMethod,
    ClassMethod,
    Constructor,
    StructMethod,
    Vector,
    SmartPtr,
    Ref,
    Primitive,
    Pointer,
    HashMap,
    Field
}
public abstract class TypeDefinition(TypeDefKind kind)
{
    public string Name { get; set; } = string.Empty;
    public string Comment { get; set; } = string.Empty;
    public string HeaderFilePath { get; set; } = string.Empty;
    public virtual TypeDefKind Kind => kind;
    public abstract string GetUniqueName();
}