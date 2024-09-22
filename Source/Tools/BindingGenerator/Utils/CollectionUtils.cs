namespace BindingGenerator.Utils;

public static class CollectionUtils
{
    public static IEnumerable<TSource> NonRepeated<TSource, TKey>(this IEnumerable<TSource> collection, Func<TSource, TKey> selector)
    {
        return collection
            .GroupBy(selector)
            .Select(x => x.First());
    }
}