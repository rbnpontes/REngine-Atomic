namespace BindingGenerator.Models;

public class PointerTypeDefinition(Module module, ModuleItem? moduleItem, TypeDefinition type) : TypeDefinition(module, moduleItem)
{
    public TypeDefinition Type => type;
    public override string GetUniqueName()
    {
        return type.GetUniqueName() + "*";
    }
}