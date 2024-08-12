namespace BindingGenerator.Models;

public class ConstructorMethodDefinition(ClassDefinition owner)
    : BaseMethodDefinition(TypeDefKind.Constructor)
{
    public ClassDefinition Owner { get; } = owner;
    public override string GetUniqueName()
    {
        return "ctor "+Owner.GetUniqueName() + "::" + base.GetUniqueName();
    }
}
public class ClassMethodDefinition(ClassDefinition owner) : BaseMethodDefinition(TypeDefKind.ClassMethod)
{
    public ClassDefinition Owner { get; } = owner;
    public bool IsStatic { get; set; }
    public override string GetUniqueName()
    {
        return Owner.GetUniqueName() + "::" + base.GetUniqueName();
    }
}
public class StructMethodDefinition(StructDefinition owner) : BaseMethodDefinition(TypeDefKind.StructMethod)
{
    public StructDefinition Owner { get; } = owner;
    public override string GetUniqueName()
    {
        return Owner.GetUniqueName() + "::" + base.GetUniqueName();
    }
}

public class StaticMethodDefinition(NamespaceDefinition @namespace) : BaseMethodDefinition(TypeDefKind.StaticMethod)
{
    public NamespaceDefinition Namespace { get; } = @namespace;

    public override string GetUniqueName()
    {
        return Namespace.GetUniqueName() + "::" + base.GetUniqueName();
    }
}