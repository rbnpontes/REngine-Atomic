using System.Text.Json;
using System.Text.Json.Serialization;
using BindingGenerator.Models;

namespace BindingGenerator;

public class TypeDefinitionDeserializer
{
    private TypeDefSerializeData[] pTypes = [];
    private NamespaceDefinition? pRootNamespace;

    private Dictionary<TypeDefSerializeData, TypeDefinition> pTypesMap = new();

    public NamespaceDefinition Namespace => pRootNamespace ?? throw new NullReferenceException();
    public int TotalOfTypes => pTypes.Length;

    public void Deserialize(Stream stream)
    {
        pRootNamespace = null;
        using var reader = new StreamReader(stream);
        pTypes = JsonSerializer.Deserialize<TypeDefSerializeData[]>(reader.ReadToEnd()) ??
                 throw new NullReferenceException();


        foreach (var type in pTypes)
        {
        }
    }
}