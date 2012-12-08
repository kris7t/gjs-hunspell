
const Hun = imports.hunspell;

let spell = new Hun.Spell("/usr/share/hunspell/hu_HU.aff",
			  "/usr/share/hunspell/hu_HU.dic");

print("version = " + spell.version);
print("dic_encoding = " + spell.dic_encoding);
print("wordchars = " + spell.wordchars);

print("\nspell arvizturokkel:");
print(spell.spell("arvizturokkel"));

print("\nspell árvíztűrőkkel:");
// Hun.Spell can be invoked as a function
foo = spell("árvíztűrőkkel");
for (let x in foo) {
    print(x + " = " + foo[x]);
}

print("\nsuggest arvizturokkel:");
print(spell.suggest("arvizturokkel"));

print("\nsuggest árvíztűrőkkel:");
print(spell.suggest("árvíztűrőkkel"));

spell = new Hun.Spell("/usr/share/hunspell/en_US.aff",
		      "/usr/share/hunspell/en_US.dic");

print("\nanalyze foxes:");
print(spell.analyze("foxes"));
