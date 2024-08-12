namespace BindingGenerator.Models;

public class NamespaceDefinition() : TypeDefinition(TypeDefKind.Namespace)
{
    public NamespaceDefinition? Owner { get; set; }
    public EnumDefinition[] Enums { get; set; } = [];
    public ClassDefinition[] Classes { get; set; } = [];
    public StructDefinition[] Structs { get; set; } = [];
    public StaticMethodDefinition[] Methods { get; set; } = [];
    public NamespaceDefinition[] Namespaces { get; set; } = [];

    public bool IsEmpty
    {
        get
        {
            var isEmpty = Enums.Length == 0 && Classes.Length == 0 && Methods.Length == 0;
            foreach (var @namespace in Namespaces)
            {
                if (@namespace.IsEmpty)
                    continue;
                isEmpty = false;
                break;
            }

            return isEmpty;
        }
    }

    public override string GetUniqueName()
    {
        var parentName = Owner?.GetUniqueName() ?? string.Empty;
        return parentName + "::" + Name;
    }
}

public class GlobalNamespaceDefinition : NamespaceDefinition
{
    public override string GetUniqueName()
    {
        return "global";
    }
}