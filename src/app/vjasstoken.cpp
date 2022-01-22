#include <exception>

#include <QtCore>

#include "vjasstoken.h"

// items are taken from https://github.com/tdauth/syntaxhighlightings/blob/master/Kate/vjass.xml

const QString VJassToken::KEYWORD_ENDFUNCTION = "endfunction";
const QString VJassToken::KEYWORD_FUNCTION = "function";
const QString VJassToken::KEYWORD_TAKES = "takes";
const QString VJassToken::KEYWORD_NOTHING = "nothing";
const QString VJassToken::KEYWORD_RETURNS = "returns";
const QString VJassToken::KEYWORD_RETURN = "return";
const QString VJassToken::KEYWORD_SET = "set";
const QString VJassToken::KEYWORD_CALL = "call";
const QString VJassToken::KEYWORD_IF = "if";
const QString VJassToken::KEYWORD_THEN = "then";
const QString VJassToken::KEYWORD_ELSEIF = "elseif";
const QString VJassToken::KEYWORD_ENDIF = "endif";
const QString VJassToken::KEYWORD_LOOP = "loop";
const QString VJassToken::KEYWORD_ENDLOOP = "endloop";
const QString VJassToken::KEYWORD_EXITWHEN = "exitwhen";
const QString VJassToken::KEYWORD_GLOBALS = "globals";
const QString VJassToken::KEYWORD_ENDGLOBALS = "endglobals";
const QString VJassToken::KEYWORD_CONSTANT = "constant";
const QString VJassToken::KEYWORD_TYPE = "type";
const QString VJassToken::KEYWORD_EXTENDS = "extends";
const QString VJassToken::KEYWORD_NATIVE = "native";
const QString VJassToken::KEYWORD_NULL = "null";

const QStringList VJassToken::KEYWRODS_ALL = {
    VJassToken::KEYWORD_ENDFUNCTION, // match before function
    VJassToken::KEYWORD_FUNCTION,
    VJassToken::KEYWORD_TAKES,
    VJassToken::KEYWORD_NOTHING,
    VJassToken::KEYWORD_RETURNS,
    VJassToken::KEYWORD_RETURN,
    VJassToken::KEYWORD_SET,
    VJassToken::KEYWORD_CALL,
    VJassToken::KEYWORD_IF,
    VJassToken::KEYWORD_THEN,
    VJassToken::KEYWORD_ELSEIF,
    VJassToken::KEYWORD_ENDIF,
    VJassToken::KEYWORD_LOOP,
    VJassToken::KEYWORD_ENDLOOP,
    VJassToken::KEYWORD_EXITWHEN,
    VJassToken::KEYWORD_GLOBALS,
    VJassToken::KEYWORD_ENDGLOBALS,
    VJassToken::KEYWORD_CONSTANT,
    VJassToken::KEYWORD_TYPE,
    VJassToken::KEYWORD_EXTENDS,
    VJassToken::KEYWORD_NATIVE,
    VJassToken::KEYWORD_NULL
};

const QSet<QString> VJassToken::COMMONJ_TYPES_ALL = {
    "integer",
    "string",
    "real",
    "boolean",
    "code",
    "handle",
    "agent",
    "unit",
    "timer",
    "group",
    "trigger",
    "player",
    "rect",
    "location",
    "ability",
    "aidifficulty ",
    "alliancetype",
    "attacktype",
    "blendmode",
    "boolexpr",
    "buff",
    "button",
    "camerafield",
    "camerasetup",
    "conditionfunc",
    "damagetype",
    "defeatcondition",
    "destructable",
    "dialog",
    "dialogevent",
    "effect",
    "effectype",
    "event",
    "eventid",
    "fgamestate",
    "filterfunc",
    "fogmodifier",
    "fogstate",
    "force",
    "gamecache",
    "gamedifficulty",
    "gameevent",
    "gamespeed",
    "gamestate",
    "gametype",
    "igamestate",
    "image",
    "item",
    "itempool",
    "itemtype",
    "leaderboard",
    "lightning",
    "limitop",
    "mapcontrol",
    "mapdensity",
    "mapflag",
    "mapsetting",
    "mapvisibility",
    "multiboard",
    "multiboarditem",
    "pathingtype",
    "placement",
    "playercolor",
    "playerevent",
    "playergameresult",
    "playerslotstate ",
    "playerscore",
    "playerstate",
    "playerunitevent",
    "quest",
    "questitem",
    "race",
    "racepreference",
    "raritycontrol",
    "rect",
    "region",
    "sound",
    "soundtype",
    "startlocprio",
    "terraindeformation",
    "texmapflags",
    "texttag",
    "timerdialog",
    "trackable",
    "triggeraction",
    "triggercondition",
    "ubersplat",
    "unitevent",
    "unitpool",
    "unitstate",
    "unittype",
    "version",
    "volumegroup",
    "weapontype",
    "weathereffect",
    "effecttype",
    "widget",
    "widgetevent",
    "hashtable",
    "nothing ",
    "array",
    "thistype",
    // Reforged
    "minimapicon",
    "mousebuttontype",
    "animtype",
    "subanimtype",
    "framehandle",
    "originframetype",
    "framepointtype",
    "textaligntype",
    "frameeventtype",
    "oskeytype",
    "abilityintegerfield",
    "abilityrealfield",
    "abilitybooleanfield",
    "abilitystringfield",
    "abilityintegerlevelfield",
    "abilityreallevelfield",
    "abilitybooleanlevelfield",
    "abilitystringlevelfield",
    "abilityintegerlevelarrayfield",
    "abilityreallevelarrayfield",
    "abilitybooleanlevelarrayfield",
    "abilitystringlevelarrayfield",
    "unitintegerfield",
    "unitrealfield",
    "unitbooleanfield",
    "unitstringfield",
    "unitweaponintegerfield",
    "unitweaponrealfield",
    "unitweaponbooleanfield",
    "unitweaponstringfield",
    "itemintegerfield",
    "itemrealfield",
    "itembooleanfield",
    "itemstringfield",
    "movetype",
    "targetflag",
    "armortype",
    "heroattribute",
    "defensetype",
    "regentype",
    "unitcategory",
    "pathingflag",
    "commandbuttoneffect"
};
const QSet<QString> VJassToken::COMMONJ_NATIVES_ALL = {
    "ConvertRace"

};
const QSet<QString> VJassToken::COMMONJ_CONSTANTS_ALL = {
    "FALSE"
};

VJassToken::VJassToken(const QString &value, int line, int column, Type type)
    : value(value)
    , line(line)
    , column(column)
    , type(type)
    , valueLengthCached(value.length())
    , isCommonJTypeCached(COMMONJ_TYPES_ALL.contains(value))
    , isCommonJNativeCached(COMMONJ_NATIVES_ALL.contains(value))
    , isCommonJConstantCached(COMMONJ_CONSTANTS_ALL.contains(value))
{
}

const QString& VJassToken::getValue() const {
    return value;
}

int VJassToken::getLine() const {
    return line;
}

int VJassToken::getColumn() const {
    return column;
}

VJassToken::Type VJassToken::getType() const {
    return type;
}

bool VJassToken::isValidType() const {
    QSet<QString> standardTypes;
    // TODO parse natives instead of adding them here
    standardTypes.insert("integer");
    standardTypes.insert("real");
    standardTypes.insert("boolean");
    standardTypes.insert("unit");

    return standardTypes.contains(getValue());
}

bool VJassToken::isValidIdentifier() const {
    return QRegularExpression("[a-zA-Z]{1}[a-zA-Z0-9]*").match(getValue()).hasMatch() && !VJassToken::KEYWRODS_ALL.contains(getValue());
}

bool VJassToken::isValidKeyword() const {
    return getType() == VJassToken::EndfunctionKeyword
        || getType() == VJassToken::FunctionKeyword
        || getType() == VJassToken::TakesKeyword
        || getType() == VJassToken::NothingKeyword
        || getType() == VJassToken::ReturnsKeyword
        || getType() == VJassToken::ReturnKeyword
        || getType() == VJassToken::SetKeyword
        || getType() == VJassToken::CallKeyword
        || getType() == VJassToken::IfKeyword
        || getType() == VJassToken::ThenKeyword
        || getType() == VJassToken::ElseifKeyword
        || getType() == VJassToken::EndifKeyword
        || getType() == VJassToken::LoopKeyword
        || getType() == VJassToken::EndloopKeyword
        || getType() == VJassToken::ExitwhenKeyword
        || getType() == VJassToken::ConstantKeyword
        || getType() == VJassToken::TypeKeyword
        || getType() == VJassToken::ExtendsKeyword
        || getType() == VJassToken::NativeKeyword
        || getType() == VJassToken::GlobalsKeyword
        || getType() == VJassToken::EndglobalsKeyword
        || getType() == VJassToken::NullKeyword
    ;
}

VJassToken::Type VJassToken::typeFromKeyword(const QString &keyword) {
    if (keyword == KEYWORD_ENDFUNCTION) {
        return VJassToken::EndfunctionKeyword;
    } else if (keyword == KEYWORD_FUNCTION) {
        return VJassToken::FunctionKeyword;
    } else if (keyword == KEYWORD_TAKES) {
        return VJassToken::TakesKeyword;
    } else if (keyword == KEYWORD_NOTHING) {
        return VJassToken::NothingKeyword;
    } else if (keyword == KEYWORD_RETURNS) {
        return VJassToken::ReturnsKeyword;
    } else if (keyword == KEYWORD_RETURN) {
        return VJassToken::ReturnKeyword;
    } else if (keyword == KEYWORD_SET) {
        return VJassToken::SetKeyword;
    } else if (keyword == KEYWORD_CALL) {
        return VJassToken::CallKeyword;
    } else if (keyword == KEYWORD_IF) {
        return VJassToken::IfKeyword;
    } else if (keyword == KEYWORD_THEN) {
        return VJassToken::ThenKeyword;
    } else if (keyword == KEYWORD_ELSEIF) {
        return VJassToken::ElseifKeyword;
    } else if (keyword == KEYWORD_ENDIF) {
        return VJassToken::EndifKeyword;
    } else if (keyword == KEYWORD_LOOP) {
        return VJassToken::LoopKeyword;
    } else if (keyword == KEYWORD_ENDLOOP) {
        return VJassToken::EndloopKeyword;
    } else if (keyword == KEYWORD_EXITWHEN) {
        return VJassToken::ExitwhenKeyword;
    } else if (keyword == KEYWORD_CONSTANT) {
        return VJassToken::ConstantKeyword;
    } else if (keyword == KEYWORD_TYPE) {
        return VJassToken::TypeKeyword;
    } else if (keyword == KEYWORD_EXTENDS) {
        return VJassToken::ExtendsKeyword;
    } else if (keyword == KEYWORD_NATIVE) {
        return VJassToken::NativeKeyword;
    } else if (keyword == KEYWORD_GLOBALS) {
        return VJassToken::GlobalsKeyword;
    } else if (keyword == KEYWORD_ENDGLOBALS) {
        return VJassToken::EndglobalsKeyword;
    } else if (keyword == KEYWORD_NULL) {
        return VJassToken::NullKeyword;
    }

    Q_ASSERT(false);
}

int VJassToken::getValueLength() const {
    return valueLengthCached;
}

bool VJassToken::isCommonJType() const {
    return isCommonJTypeCached;
}

bool VJassToken::isCommonJNative() const {
    return isCommonJNativeCached;
}

bool VJassToken::isCommonJConstant() const {
    return isCommonJConstantCached;
}
