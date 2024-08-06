namespace BindingGenerator.Models;

public class TypeDefinition(Module module, ModuleItem moduleItem) : BaseDefinition(module, moduleItem)
{
    public string Name { get; set; } = string.Empty;
}