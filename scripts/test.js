const testFile = process.argv[2];
const tests = require(testFile);

const assert = require('assert');

Object.keys(tests).forEach(test => {
  if (test.startsWith('test_')) {
    console.error('Running: ' + test);
    tests[test](assert);
  }
});
