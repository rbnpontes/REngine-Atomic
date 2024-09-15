// Simple Test Suite
const TEST_CASES = [];
var CURR_DESC = null;

function describe(desc, call) {
    if (CURR_DESC == null)
        CURR_DESC = { name: desc, parent: null };
    else
        CURR_DESC = { name: desc, parent: CURR_DESC };

    call();
    CURR_DESC = CURR_DESC.parent;
}

function it(testCase, call) {
    if (CURR_DESC == null)
        throw new Error("Requires call 'describe' first.");

    var name = testCase;
    var desc = CURR_DESC;
    // build test case names
    while (desc) {
        name = desc.name + " > " + name;
        desc = desc.parent;
    }

    TEST_CASES.push([name, call]);
}

function runTests() {
    var nextTestId = -1;
    var failedTests = [];
    var successTests = [];

    var execTestEnd = function () {
        successTests.forEach(function(testName) {
            console.log('- [PASS] '+testName);
        });
        failedTests.forEach(function(testName, err) {
            console.error('- [FAILED] '+testName, err);
        });
        console.log('- Finished Tests Execution');
        console.log('- Success: '+successTests.length +' - Failed: '+failedTests.length);
    };
    var handleSuccessTest = function (testName) {
        successTests.push([testName]);
    };
    var handleFailedTest = function (testName, err) {
        failedTests.push([testName, err]);
        console.error('- [FAILED]: ' + testName, err);
    };
    var executeTest = function (testName, testCall, callback) {
        const completeFn = function () {
            handleSuccessTest(testName);
            callback();
        };
        const errorFn = function (err) {
            handleFailedTest(testName, err);
        };

        var resultFn;
        try {
            resultFn = testCall();
        } catch (e) {
            errorFn(e);
            return;
        }

        if (typeof resultFn !== 'function') {
            completeFn();
            return;
        }

        try {
            resultFn(completeFn, errorFn);
        } catch (e) {
            errorFn(e);
        }
    };

    var execNextTest = function () {
        ++nextTestId;

        if (nextTestId == TEST_CASES.length) {
            execTestEnd();
            return;
        }

        const pair = TEST_CASES[nextTestId];
        const testName = pair[0];
        const testCall = pair[1];

        console.log('- [RUNNING]: ' + testName);

        // if test call results in a function
        // we must pass a resolve method as arg and call
        executeTest(testName, testCall, execNextTest);
    };

    console.log('.:: Running Engine Sandbox Javascript Tests ::.');
    console.log('- Running ' + TEST_CASES.length + ' tests.');
    execNextTest();
}

// Test Cases Above

describe('Logging', function () {
    it('must log with console.log', function () {
        console.log('test log info');
    });
    it('must log with console.warn', function () {
        console.warn('test warning log');
    });
    it('must log with console.debug', function () {
        console.debug('test debug log');
    });
    it('must log with console.error', function () {
        console.error('test error log');
    });
});


describe('Require/Module Loader', function () {
    const reqFn = require;
    it('require must be defined.', function () {
        if (typeof reqFn == 'undefined')
            throw new Error('Require function is not available.');
    });
    it('require must contains cache property.', function () {
        if (typeof reqFn.cache == 'undefined')
            throw new Error('Require function doesn\'t contains cache object.');
    });
    it('must load a javascript file.', function () {
        const res = require('./test-module.js');
        console.log('Result:', res);
    });
    it('must load and cache javascript exports.', function () {
        const export_0 = require('./test-module.js');
        const export_1 = require('./test-module.js');

        if (export_0 != export_1)
            throw new Error('Invalid require implementation. Require must return same reference of previous loaded module!');
    });
});


describe('Timers', function () {
    it('must be defined basic timer calls.', function () {
        const immediateFn = setImmediate;
        const timeoutFn = setTimeout;
        const intervalFn = setInterval;

        if (typeof immediateFn === 'undefined')
            throw new Error('requires setImmediate');
        if (typeof timeoutFn === 'undefined')
            throw new Error('requires setTimeout');
        if (typeof intervalFn === 'undefined')
            throw new Error('requires setInterval');
    });
    it('must schedule call with immediate.', function () {
        return function (resolve) {
            setImmediate(resolve);
        };
    });
    it('must schedule call with timeout.', function () {
        return function (resolve) {
            setTimeout(resolve, 1000);
        };
    });
    it('must schedule calls with 5 intervals.', function () {
        return function (resolve) {
            var count = 0;
            const timer = setInterval(function () {
                ++count;
                if (count < 5)
                    return;
                clearInterval(timer);
                resolve();
            }, 10);
        };
    });
    it('must clear immediate.', function () {
        const errorFn = function () {
            throw new Error('immediate was not cleared.');
        };
        const timer = setImmediate(errorFn);
        clearImmediate(timer);
    });
    it('must clear timeout.', function () {
        const errorFn = function () {
            throw new Error('timeout was not cleared.');
        };
        const timer = setTimeout(errorFn, 100);
        clearTimeout(timer);
        return function (resolve) {
            setTimeout(resolve, 200);
        };
    });
});


// execute tests.
runTests();