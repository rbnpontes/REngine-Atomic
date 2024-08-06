namespace BindingGenerator.Models;

public class IntegerDefinition(Module module, ModuleItem moduleItem) : TypeDefinition(module, moduleItem)
{
    public override string GetUniqueName()
    {
        throw new NotImplementedException();
    }
}