#Πτυχιακή εργασία

Θέμα: *Μελέτη και υλοποίηση συστήματος αυτοματισμού
για την απομακρυσμένη παρακολούθηση
περιβαλλοντικών συνθηκών*


##Γενικά


Τεχνολογικό Εκπαιδευτικό Ίδρυμα Αθήνας  
Σχολή Τεχνολογικών Εφαρμογών  
Τμήμα Πληροφορικής

Επιμέλεια  
Θεόδωρος Ελευθέριος Πάνου – ΑΜ 071045

Επίβλεψη  
Ιφιγένεια Φουντά


##Περίληψη
Η εργασία ασχολείται με την υλοποίηση μίας συσκευής που αυτοματοποιεί την
παρακολούθηση συνθηκών που επικρατούν στο υλικό ανοικτού δοχείου το οποίο αυτή
περιβάλλει. Η πρόσβαση στη συσκευή πραγματοποιείται απομακρυσμένα με χρήση
πρωτοκόλλου HTTP.

Η υλοποίηση στηρίζεται στη χρήση μίας πλακέτας Arduino Uno rev3 και ενός
μικροελεγκτή Atmel AVR ATmega328 που αυτή περιέχει. Ο προγραμματισμός του
μικροελεγκτή γίνεται δια μέσω λογισμικού Boot loader και διασύνδεσης USB.
Η ανάπτυξη του λογισμικού υποβοηθείται μόνο από την AVR GNU συλλογή
μεταγλωττιστών avr-gcc και τη βιβλιοθήκη προγραμματισμού AVR Libc. Κρίσιμες
δυνατότητες που ο συγκεκριμένος μικροελεγκτής στερείται, όπως ρολόι πραγματικού
χρόνου, δικτυακή διασύνδεση και επιπρόσθετο αποθηκευτικό χώρο, παρέχονται από
εξωτερικά ολοκληρωμένα κυκλώματα.

Η συσκευή ρυθμίζεται σε σχέση με παραμέτρους βοηθητικών λειτουργιών της καθώς
και της διαδικασίας παρακολούθησης. Όλες οι ρυθμίσεις διατηρούνται μεταξύ
διακοπών τροφοδοσίας και είναι δυνατό να επαναφερθούν στις εργοστασιακές
της ρυθμίσεις με μηχανικό τρόπο.

Για τη διασύνδεσή της με το χρήστη, υλοποιείται λογισμικό διακομιστή HTTP μέσω
του οποίου επιτυγχάνεται η ανάκτηση πόρων, οι οποίοι, σε συνδυασμό με
παρεχόμενες μεθόδους του πρωτοκόλλου HTTP, επιτρέπουν πρόσβαση σε αυτήν.
Η μορφή των αναπαραστάσεων των πόρων γίνεται σε JSON και προορίζεται για
αξιοποίηση από εξωτερικές εφαρμογές. Επιπλέον, υποστηρίζονται πόροι σε HTML,
CSS, JavaScript και PNG για χρήση της συσκευής μέσω λογισμικού πλοήγησης.

Το κεντρικό σημείο ενδιαφέροντος της εργασίας έγκειται στην υλοποίηση ενός
ολοκληρωμένου συστήματος που εμπλέκει πληθώρα τεχνολογικών πεδίων.


##Παραδοτέα

Ιδιαίτερου ενδιαφέροντος είναι τα αρχεία

* senior-thesis / doc /
[main.pdf](https://github.com/tpanou/senior-thesis/blob/master/doc/main.pdf)
* senior-thesis / doc /
[beamer.pdf](https://github.com/tpanou/senior-thesis/blob/master/doc/beamer.pdf)

τα οποία αποτελούν το έγγραφο και την παρουσίαση της εργασίας, αντίστοιχα.