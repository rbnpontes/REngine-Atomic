namespace BindingGenerator.Models;

public class EnumEntry(EnumDefinition owner)
{
    public EnumDefinition Owner { get; } = owner;
    public string Comment { get; set; } = string.Empty;
    public string Name { get; set; } = string.Empty;
    public int Value { get; set; }
}
public class EnumDefinition(Module module, ModuleItem moduleItem, NamespaceDefinition @namespace) : TypeDefinition(module, moduleItem)
{
    public EnumEntry[] Entries { get; set; } = [];
    public NamespaceDefinition Namespace { get; } = @namespace;
    public override string GetUniqueName()
    {
        return Namespace.GetUniqueName() + "::" + Name;
    }
}