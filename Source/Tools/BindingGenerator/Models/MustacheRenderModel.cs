namespace BindingGenerator.Models;

class SourceMustacheModel(Module module, ModuleItem current)
{
    public Module Module { get; set; } = module;
    public ModuleItem Current { get; set; } = current;
}