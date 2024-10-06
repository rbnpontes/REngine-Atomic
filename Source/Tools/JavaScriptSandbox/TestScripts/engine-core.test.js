describe('Enums', function() {
    it('should load basic engine enums.', function() {
        if(typeof LoopMode2D === 'undefined')
            throw new Error('LoopMode2D is not defined');
        console.log('LoopMode2D:', LoopMode2D);
        if(typeof Atomic.CullMode === 'undefined')
            throw new Error('CullMode is not defined');
        console.log('CullMode:', Atomic.CullMode);
    });
});
describe('Structs', function () {
    it('should instantiate struct.', function () {
        const data = new Atomic.ResourceRef();
        console.log('Resource Ref:', data);
    });
    it('should crash if instatiation is without new keyword.', function () {
        const instantiate = function () {
            try {
                Atomic.ResourceRef();
                return true;
            } catch (e) {
                console.error(e);
                return false;
            }
        };

        if (instantiate())
            throw new Error('test failed');
    });
});