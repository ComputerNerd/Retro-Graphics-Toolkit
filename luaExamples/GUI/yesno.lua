local ret=fl.ask("Press yes or no")
if ret~=0 then
	fltk.message("Yes")
else
	fltk.message("No")
end
