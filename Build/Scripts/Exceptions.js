function getUnsupportedEnvironmentError(){
    return new Error('Unsupported environment. This OS is not supported!');
}

module.exports = {
    getUnsupportedEnvironmentError
};