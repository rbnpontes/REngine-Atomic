namespace BindingGenerator.Models;

public abstract class TypeDefinition(Module module, ModuleItem? moduleItem) : BaseDefinition(module, moduleItem)
{
    public string Name { get; set; } = string.Empty;
    public string Comment { get; set; } = string.Empty;
    public string HeaderFilePath { get; set; } = string.Empty;
    public abstract string GetUniqueName();
}