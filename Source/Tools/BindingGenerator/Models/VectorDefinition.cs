using CppAst;

namespace BindingGenerator.Models;

public enum VectorType
{
    Undefined = 0,
    Default,
    Pod
}
public class VectorDefinition(TypeDefinition type, VectorType vecType) : TypeDefinition(TypeDefKind.Vector)
{
    private static readonly Dictionary<string, VectorType> pVecTypes = new ()
    {
        { "vector", VectorType.Default },
        { "podvector", VectorType.Pod } 
    };
    
    public TypeDefinition ElementType => type;
    public VectorType Type => vecType;
    public override string GetUniqueName()
    {
        return type.GetUniqueName() + "[]";
    }
    
    public static bool IsVector(CppType type)
    {
        return GetVectorType(type) != VectorType.Undefined;
    }

    public static VectorType GetVectorType(CppType type)
    {
        if (pVecTypes.TryGetValue(type.GetDisplayName().ToLowerInvariant(), out var vecType))
            return vecType;
        return VectorType.Undefined;
    }
}