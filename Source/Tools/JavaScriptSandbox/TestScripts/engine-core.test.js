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