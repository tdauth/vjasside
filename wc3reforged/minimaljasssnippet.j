// Minimal JASS snippet with all kinds of expressions, statements and nestings.
globals
    constant integer MY_NUMBER = (10 + 10000 - 5) / 100 * 20
    string array myTexts
    real myReal = 2.0
    location myLocation = Location(0.0, 0.0)
    constant integer MY_RAWCODE = 'hmpr'
endglobals

constant function MyConstantFunction takes integer x returns string
    return "Hello\nWorld!"
endfunction

function MyFunction takes string text, code myFunc returns nothing
    local integer i = 0
    set i = 0
    if (true and true and not (MyConstantFunction(10) != "Test" or 10 < 100 or 10 > 2 or 3 <= 3 or 3 >= 3 or 3 != 2)) then
        loop
            exitwhen i > 100 or (i == 100)
                call DisplayTextToPlayer(Player(0), 0.0, 0.0, "Number " + I2S(i))
            set i = i + 1
        endloop
    elseif true then
        return
    else
        return
    endif
endfunction