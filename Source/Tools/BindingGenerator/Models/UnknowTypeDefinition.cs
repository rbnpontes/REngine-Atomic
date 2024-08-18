namespace BindingGenerator.Models;

public class UnknowTypeDefinition() : TypeDefinition(TypeDefKind.Unknow)
{
    public static readonly UnknowTypeDefinition Default = new();
    public override string GetUniqueName()
    {
        throw new Exception("Unknow Type cannot be used.");
    }
}