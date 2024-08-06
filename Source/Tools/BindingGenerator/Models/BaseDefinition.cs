namespace BindingGenerator.Models;

public class BaseDefinition(Module module, ModuleItem moduleItem)
{
    public Module Module { get; set; } = module;
    public ModuleItem ModuleItem { get; set; } = moduleItem;
}