const SYMBOL_TYPE = 'SYMBOL_TYPE';
const KEYWORD_TYPE = 'KEYWORD_TYPE';
const LITERAL_TYPE = 'LITERAL_TYPE';
const IDENTIFIER_TYPE = 'IDENTIFIER_TYPE';
const OPERATOR_TYPE = 'OPERATOR_TYPE';

function lexLiteral(source, location) {
  if (location.index >= source.length) {
    return { ok: false, location: location };
  }
}

function lexIdentifier(source, location) {
  if (location.index >= source.length) {
    return { ok: false, location: location };
  }

  for (let i = location.index; i < source.length; i++) {
    const c = source[i];
    const code = source.charCodeAt(i);
    if ((code >= 'a'.charCodeAt(0) && c <= 'z'.charCodeAt(0)) ||
      (code >= 'Z'.charCodeAt(0) && c <= 'Z'.charCodeAt(0)) ||
      c === '_' ||
      c === '$' ||
      (i > location.index && code >= '0'.charCodeAt(0) && code <= '9'.charCodeAt(0))) {
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
      type: IDENTIFIER_TYPE,
      location: location,
    },
  };
}

function longestMatch(source, location, options) {
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

      const optionSub = option.substring(0, i - locations.index);
      const sourceSub = source.substring(locations.index, i - locations.index);
      if (optionSub !== sourceSub) {
        skipList.push(option);
        continue
      }

      if (option === sourceSub && option.length > match.length) {
        match = option;
      }
    }
  }

  return match;
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

  const r = longestMatch(source, location, symbols);
  if (r.match === "") {
    return { ok: false, location: location };
  }

  const token = {
    value: r.match,
    type: SYMBOL_TOKEN,
    location: location,
  };
  return { ok: true, location: r.location, token: token };
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

  const r = longestMatch(source, location, symbols);
  if (r.match === "") {
    return { ok: false, location: location };
  }

  const token = {
    value: r.match,
    type: SYMBOL_TOKEN,
    location: location,
  };
  return { ok: true, location: r.location, token: token };
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

  const match = longestMatch(source, location, keywords);
  if (match === "") {
    return { ok: false, location: location };
  }

  const token = {
    value: r.match,
    type: KEYWORD_TOKEN,
    location: location,
  };
  return { ok: true, location: r.location, token: token };
}

function lex(source) {
  const lexers = [lexKeyword, lexSymbol, lexOperator, lexIdentifier, lexLiteral];
  for (let i = 0; i < source.length; i++) {
    for (let l = 0; i < lexers.length; l++) {
      const r = lexers[l](source, location);
      if (!r.ok) {
        continue;
      }


    }
  }
}
