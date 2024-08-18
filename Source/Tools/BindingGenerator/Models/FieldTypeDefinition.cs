namespace BindingGenerator.Models;

public class StructFieldTypeDefinition(StructDefinition owner) : TypeDefinition(TypeDefKind.Field)
{
    public bool IsStatic { get; set; }
    public StructDefinition Owner => owner;
    public TypeDefinition Type { get; set; } = UnknowTypeDefinition.Default;
    public override string GetUniqueName()
    {
        return $"{Type.GetUniqueName()} {owner.GetUniqueName()}::{Name}";
    }
}

public class ClassFieldTypeDefinition(ClassDefinition owner) : TypeDefinition(TypeDefKind.Field)
{
    public bool IsStatic { get; set; }
    public ClassDefinition Owner => owner;
    public TypeDefinition Type { get; set; } = UnknowTypeDefinition.Default;
    public override string GetUniqueName()
    {
        return $"{Type.GetUniqueName()} {owner.GetUniqueName()}::{Name}";
    }
}