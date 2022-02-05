globals
    location whichLocation
    rect whichRect
    unit whichUnit
endglobals

function main takes nothing returns nothing
	call RemoveLocation(whichLocation)
	call RemoveRect(whichRect)
	call RemoveUnit(whichUnit)
endfunction