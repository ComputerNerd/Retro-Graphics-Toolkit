-- fl.choice allows up to three choices
e=fl.choice("Question","You have no choice")
fl.message(e)
e=fl.choice("Question","Zero","One")
fl.message(e)
e=fl.choice("Question","Zero","One","Two")
fl.message(e)
