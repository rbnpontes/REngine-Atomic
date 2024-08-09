namespace BindingGenerator.Models;

public enum VectorType
{
    Undefined = 0,
    Default,
    Pod
}
public class VectorDefinition(Module module, ModuleItem? moduleItem, TypeDefinition type, VectorType vecType) : TypeDefinition(module, moduleItem)
{
    public TypeDefinition ElementType => type;
    public VectorType Type => vecType;
    public override string GetUniqueName()
    {
        return type.GetUniqueName() + "[]";
    }
}