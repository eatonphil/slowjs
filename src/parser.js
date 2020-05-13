function parseFunction(tokens, index) {
  if (index >= tokens.length) {
    return { ok: false };
  }

  // TODO: drop destructuring

  let ok;
  ({ index, ok } = parseToken('function'));
  if (!ok) {
    return { ok: false };
  }

  let token;
  ({ index, token, ok } = parseTokenType(IDENTIFIER_TYPE));
  if (!ok) {
    return { ok: false };
  }

  ({ index, });
}

module.exports.parse = function (tokens) {

};
