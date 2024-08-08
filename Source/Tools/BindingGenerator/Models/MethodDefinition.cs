namespace BindingGenerator.Models;

public class ClassMethodDefinition(Module module, ModuleItem moduleItem, ClassDefinition owner) : BaseMethodDefinition(module, moduleItem)
{
    public ClassDefinition Owner { get; } = owner;
    public override string GetUniqueName()
    {
        return Owner.GetUniqueName() + "::" + base.GetUniqueName();
    }
}
public class StructMethodDefinition(Module module, ModuleItem moduleItem, StructDefinition owner) : BaseMethodDefinition(module, moduleItem)
{
    public StructDefinition Owner { get; } = owner;
    public override string GetUniqueName()
    {
        return Owner.GetUniqueName() + "::" + base.GetUniqueName();
    }
}

public class StaticMethodDefinition(Module module, ModuleItem moduleItem, NamespaceDefinition @namespace) : BaseMethodDefinition(module, moduleItem)
{
    public NamespaceDefinition Namespace { get; } = @namespace;

    public override string GetUniqueName()
    {
        return Namespace.GetUniqueName() + "::" + base.GetUniqueName();
    }
}