namespace BindingGenerator.Models;

public class NamespaceDefinition(Module module, ModuleItem? moduleItem) : TypeDefinition(module, moduleItem)
{
    public NamespaceDefinition? Owner { get; set; }
    public EnumDefinition[] Enums { get; set; } = [];
    public ClassDefinition[] Classes { get; set; } = [];
    public StaticMethodDefinition[] Methods { get; set; } = [];
    public NamespaceDefinition[] Namespaces { get; set; } = [];

    public bool IsEmpty => Enums.Length == 0 && Classes.Length == 0 && Methods.Length == 0 && Namespaces.Length == 0;

    public override string GetUniqueName()
    {
        var parentName = Owner?.GetUniqueName() ?? string.Empty;
        return parentName + "::" + Name;
    }
}

public class GlobalNamespaceDefinition(Module module, ModuleItem? moduleItem) : NamespaceDefinition(module, moduleItem)
{
    public override string GetUniqueName()
    {
        return "global";
    }
}