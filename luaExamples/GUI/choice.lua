-- fltk.choice allows up to three choices.

local e = fltk.choice("Question", "You have no choice")
fltk.message(e)

e = fltk.choice("Question", "Zero", "One")
fltk.message(e)

e = fltk.choice("Question", "Zero", "One", "Two")
fltk.message(e)
