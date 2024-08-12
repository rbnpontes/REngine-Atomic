using BindingGenerator.Models;

namespace BindingGenerator;

public class TypeDefinitionSerializer(NamespaceDefinition[] @namespaces)
{
    private bool pHasBuild;
    public void Build()
    {
        if (pHasBuild)
            pHasBuild = true;
        
        
    }

    public void Save(Stream stream)
    {
        if (!pHasBuild)
            throw new InvalidOperationException("Must call build first");
    }
}