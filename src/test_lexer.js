const { lex } = require('./lexer.js');

module.exports.test_lex = function (assert) {
  assert.equal([
    'function',
    '(',
    ')',
    '{',
    'return',
    'a',
    ';',
    '}',
  ], lex('function () {return a;}').map(t => t.value));
}
