
const Hun = imports.hunspell;

print("experimental = " + Hun.Spell.experimental);

let spell = new Hun.Spell("/usr/share/hunspell/hu_HU.aff",
			  "/usr/share/hunspell/hu_HU.dic");

print("\nversion = " + spell.version);
print("dic_encoding = " + spell.dic_encoding);
print("wordchars = " + spell.wordchars);
print("langnum = " + spell.langnum);
print("lang = " + spell.lang);

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
print(spell("froben")); // => true
spell.remove("frob");
print(spell("frob")); // => false
spell.add_with_affix("frob", "ox");
print(spell("froben")); // => Object

print("\nadd another dictionary:");
print(spell("majom"));
spell.add_dic("/usr/share/hunspell/hu_HU.dic");
print(spell("majom"));
