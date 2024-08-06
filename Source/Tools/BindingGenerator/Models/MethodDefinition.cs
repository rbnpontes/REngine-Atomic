namespace BindingGenerator.Models;

public class MethodDefinition(Module module, ModuleItem moduleItem) : BaseDefinition(module, moduleItem)
{
    public string Name { get; set; } = string.Empty;
    public TypeDefinition[] ArgumentTypes { get; set; } = [];
    public TypeDefinition? ReturnType { get; set; } = null;
}