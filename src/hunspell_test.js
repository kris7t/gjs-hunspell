
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
let foxes_morph = spell.analyze("foxes");
print(foxes_morph);

print("\nstem foxes:");
print(spell.stem("foxes"));
print(spell.stem(foxes_morph));

print("\ngenerate car, foxes:");
print(spell.generate("car", "foxes"));
print(spell.generate("car", foxes_morph));

print("\nadd to dictionary:");
print(spell("frob")); // => false
spell.add("frob");
print(spell("frob")); // => Object
spell.remove("frob");
print(spell("frob")); // => false
spell.add_with_affix("frob", "car");
print(spell("frobs")); // => Object
