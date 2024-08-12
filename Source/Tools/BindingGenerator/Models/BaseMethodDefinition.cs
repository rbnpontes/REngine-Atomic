using System.Text;

namespace BindingGenerator.Models;

public abstract class BaseMethodDefinition(TypeDefKind typeDefKind) : TypeDefinition(typeDefKind)
{
    public TypeDefinition[] ArgumentTypes { get; set; } = [];
    public TypeDefinition ReturnType { get; set; } = new PrimitiveTypeDefinition(PrimitiveKind.Void);

    public override string GetUniqueName()
    {
        StringBuilder sb = new();
        sb.Append(Name);
        sb.Append("(");
        for (var i = 0; i < ArgumentTypes.Length; ++i)
        {
            sb.Append(ArgumentTypes[i].GetUniqueName());
            if (i < ArgumentTypes.Length - 1)
                sb.Append(',');
        }

        sb.Append(") : ");
        sb.Append(ReturnType.GetUniqueName());
        return sb.ToString();
    }
}