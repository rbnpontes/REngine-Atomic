namespace BindingGenerator.Models;

public class PropertyDefinition(Module module, ModuleItem moduleItem) : BaseDefinition(module, moduleItem)
{
    public string Name { get; set; } = string.Empty;
    public bool HasSet { get; set; } = false;
    public bool HasGet { get; set; } = false;
}