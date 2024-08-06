namespace BindingGenerator.Models;

public abstract class BaseMethodDefinition(Module module, ModuleItem moduleItem) : BaseDefinition(module, moduleItem)
{
    public TypeDefinition[] ArgumentTypes { get; set; } = [];
    public TypeDefinition? ReturnType { get; set; } = null;
}