namespace BindingGenerator.Models;

public class MethodDefinition(Module module, ModuleItem moduleItem, ClassDefinition owner) : BaseMethodDefinition(module, moduleItem)
{
    public ClassDefinition Owner { get; set; }
}

public class StaticMethodDefinition(Module module, ModuleItem moduleItem) : BaseMethodDefinition(module, moduleItem)
{
}