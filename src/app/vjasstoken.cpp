#include <exception>

#include <QtCore>

#include "vjasstoken.h"

// items are taken from https://github.com/tdauth/syntaxhighlightings/blob/master/Kate/vjass.xml

const QString VJassToken::KEYWORD_ENDFUNCTION = "endfunction";
const QString VJassToken::KEYWORD_FUNCTION = "function";
const QString VJassToken::KEYWORD_TAKES = "takes";
const QString VJassToken::KEYWORD_NOTHING = "nothing";
const QString VJassToken::KEYWORD_RETURNS = "returns";
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
