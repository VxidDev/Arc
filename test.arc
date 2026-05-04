VAR P = 1000
VAR r = 0.05
VAR t = 10

VAR one = 1
VAR base = one + r

VAR growth = base ^ t
VAR amount = P * growth

amount
