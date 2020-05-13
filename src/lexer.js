const SYMBOL_TYPE = 'SYMBOL_TYPE';
const KEYWORD_TYPE = 'KEYWORD_TYPE';
const LITERAL_TYPE = 'LITERAL_TYPE';
const IDENTIFIER_TYPE = 'IDENTIFIER_TYPE';
const OPERATOR_TYPE = 'OPERATOR_TYPE';
const STRING_TYPE = 'STRING_TYPE';
const NUMBER_TYPE = 'NUMBER_TYPE';

function lexTestChars(source, location, type, test) {
  if (location.index >= source.length) {
    return { ok: false, location: location };
  }

  let i = location.index;
  for (; i < source.length; i++) {
    if (test(i, source, i === location.index)) {
      continue;
    }

    break;
  }

  if (i === location.index) {
    return { ok: false, location: location };
  }

  return {
    ok: true,
    location: {
      col: location.col + i - location.index,
      line: location.line,
      index: i,
    },
    token: {
      value: source.substring(location.index, i - location.index),
      type: type,
      location: location,
    },
  };
}

function lexNumberTest(i, source) {
  const code = source.charCodeAt(i);
  // TODO: floats
  return code >= '0'.charCodeAt(0) && code <= '9'.charCodeAt(0);
}

function lexNumber(source, location) {
  return lexTestChars(source, location, NUMBER_TYPE, lexNumberTest);
}

function lexStringTest(i, source, first) {
  // TODO: handle " delimiter
  if (first && source[i] !== '\'') {
    return false;
  }

  return (i < source.length - 1 && source[i - 1] === '\'' && source[i - 2] !== '\\');
}

function lexString(source, location) {
  const r = lexTestChars(source, location, STRING_TYPE, lexStringTest);
  if (!r.ok) {
    return r;
  }

  // Trim quotes
  r.token.value = r.token.value.substring(1, r.token.value.length - 2);
  return r;
}

function lexIdentifierTest(i, source, first) {
  const c = source[i];
  const code = source.charCodeAt(i);
  return ((code >= 'a'.charCodeAt(0) && c <= 'z'.charCodeAt(0)) ||
    (code >= 'Z'.charCodeAt(0) && c <= 'Z'.charCodeAt(0)) ||
    c === '_' ||
    c === '$' ||
    (!first && code >= '0'.charCodeAt(0) && code <= '9'.charCodeAt(0)));
}

function lexIdentifier(source, location) {
  return lexTestChars(source, location, IDENTIFIER_TYPE, lexIdentifierTest);
}

function longestMatch(source, location, options, type) {
  if (location.index >= source.length || options.length === 0) {
    return "";
  }

  let match = "";
  const skipList = [];
  for (let i = location.index; i < source.length; i++) {
    for (let o = 0; o < options.length; o++) {
      const option = options[o];

      if (i - location.index > option.length) {
        skipList.push(option);
        continue;
      }

      const optionSub = option.substring(0, i - location.index);
      const sourceSub = source.substring(location.index, i - location.index);
      if (optionSub !== sourceSub) {
        skipList.push(option);
        continue;
      }

      if (option === sourceSub && option.length > match.length) {
        match = option;
      }
    }
  }

  if (match === "") {
    return { ok: false, location: location };
  }

  const token = {
    value: match,
    type: type,
    location: location,
  };
  return {
    ok: true,
    location: {
      line: location.line,
      col: location.col + match.length,
      index: location.col + match.length,
    },
    token: token
  };
}

function lexOperator(source, location) {
  const operators = [
    '=',
    '==',
    '!=',
    '===',
    '!==',
    '>=',
    '>',
    '<',
    '<=',
    '-',
    '+',
    '!'
  ];

  return longestMatch(source, location, operators, OPERATOR_TYPE);
}

function lexSymbol(source, location) {
  const symbols = [
    '[',
    ']',
    ',',
    '{',
    '}',
    '(',
    ')',
    ':',
    ';'
  ];

  return longestMatch(source, location, symbols, SYMBOL_TYPE);
}

function lexKeyword(source, location) {
  const keywords = [
    'function',
    'return',
    'if',
    'for',
    'const',
    'let',
    'continue',
    'break'
  ];

  return longestMatch(source, location, keywords, KEYWORD_TYPE);
}

module.exports.lex = function (source) {
  let location = { index: 0, col: 0, line: 0 };
  const tokens = [];
  const lexers = [lexKeyword, lexSymbol, lexOperator, lexIdentifier, lexNumber, lexString];
  while (location.index < source.length) {
    let found = false;
    for (let i = 0; i < lexers.length; i++) {
      const r = lexers[i](source, location);
      if (!r.ok) {
        continue;
      }

      tokens.push(r.token);
      location = r.location;
      found = true;
    }

    if (!found) {
      throw new Error('Unable to lex token near: ' + tokens[tokens.length - 1].value);
    }
  }

  return tokens;
}
