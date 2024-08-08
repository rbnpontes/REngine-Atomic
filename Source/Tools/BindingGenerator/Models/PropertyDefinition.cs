namespace BindingGenerator.Models;

public class PropertyDefinition(Module module, ModuleItem moduleItem) : TypeDefinition(module, moduleItem)
{
    public bool HasSet { get; set; } = false;
    public bool HasGet { get; set; } = false;
    public override string GetUniqueName()
    {
        throw new NotImplementedException();
    }
}