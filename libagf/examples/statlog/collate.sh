FILES="heart.txt shuttle.txt segment.txt sat.txt"

# col  value
# 1    KNN time user (s)
# 2    KNN time system
# 3    AGF train time user
# 4    AGF train time system
# 5    AGF test time user
# 6    AGF test time system
# 7    SVM train time user
# 8    SVM train time system
# 9    SVM test time user
# 10   SVM test time system
# 11   ACC train time user
# 12   ACC train time system
# 13   ACC test time user
# 14   ACC test time system
# 15   KNN accuracy
# 16   KNN U.C.
# 17   AGF accuracy
# 18   AGF U.C.
# 19   SVM accuracy
# 20   SVM U.C.
# 21   ACC accuracy
# 22   ACC U.C.
# 23   number of suport vectors

for F in $FILES; do
   ./sum_col.exe 0 1 / 2 3 / 4 5 / 6 7 / 8 9 / 10 11 / 12 13 / 14 / 15 / 16 / 17 / 18 / 19 / 20 / 21 < $F | ./calc_stats
done
