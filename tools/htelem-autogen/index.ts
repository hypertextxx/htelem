import { writeFile } from "fs";

import { idlparsed as htmlIdl } from "./webref/ed/idlparsed/html.json"
import { elements as htmlElementsRef } from "./webref/ed/elements/html.json"

const typeMap = {
    "boolean": "bool",
    "DOMString": "std::string_view",
    "USVString": "std::string_view",
    "double": "double",
    "unrestricted double": "double",
    "long": "long",
    "unsigned long": "unsigned long",
}

const skipInterfaces = new Set(["HTMLTemplateElement", "HTMLTrackElement"]);
const skipElements = new Set([]);

const htmlElements = htmlElementsRef.filter(el => !skipInterfaces.has(el.interface) && !skipElements.has(el.name))

interface Attribute {
    attrName: string
    attrType: string | string[]
}

interface IdlInterface {
    interfaceName: string
    parents: string[]
    includes: string[]
    attributes: Attribute[]
}

function makeAttributeType(a) {
    if (Array.isArray(a)) {
        return a.map(i => makeAttributeType(i))
    } else {
        return a.idlType || a;
    }
}

function makeAttrVariant(att) {
    if (Array.isArray(att)) {
        return `std::variant<${att.map(a => makeAttrVariant(a)).join(", ")}>`;
    } else {
        if (!typeMap[att]) {
            console.warn(`missing type map for ${att}`);
            return "::ht::unimplemented";
        }
        return typeMap[att];
    }
}

function getDependencies(interfaceName: string): string[] {
    let toAdd = new Set<string>();
    if (htmlIdl.idlNames[interfaceName]?.inheritance) {
        toAdd.add(htmlIdl.idlNames[interfaceName].inheritance);
    }
    htmlIdl.idlExtendedNames[interfaceName]?.forEach(ext => {
        if (ext["name"] === interfaceName && ext["inheritance"]) {
            toAdd.add(ext["inheritance"]);
        }
        if (ext["type"] === "includes") {
            toAdd.add(ext["includes"]);
        }
    })
    if (toAdd.size > 0) {
        return [...([...toAdd].flatMap(a => getDependencies(a))), ...toAdd];
    } else {
        return [];
    }
}

let indentLevel = 0;

function indent(text: string) {
    return " ".repeat(4 * indentLevel) + text;
}

type AttrSpecs = { [name: string]: Attribute[] };

function makeAttributeSpecSection(defs: IdlInterface[]) {
    let attributeSpecSection = "";

    const specs = defs.flatMap(d => d.attributes).reduce<AttrSpecs>((prev, current) => {
        prev[`${current.attrName}`] = current.attrName in prev ? [...prev[`${current.attrName}`], current] : [current];
        return prev;
    }, { });

    for (let [name, attrs] of Object.entries(specs)) {
        attributeSpecSection += indent(`inline constexpr attribute_spec<"${name}", ${[...new Set(attrs.map(t => makeAttrVariant(t.attrType)))].join(", ")}> _${name};\n`);
    }

    return attributeSpecSection;
}

function makeAttributeListSection(defs: IdlInterface[]) {
    let attributeListSection = "";
    defs.forEach(def => {
        attributeListSection += indent(`template <> struct attribute_list<"${def.interfaceName}">`);
        if (def.parents.length > 0 || def.includes.length > 0) {
            attributeListSection += `: ${[...def.parents, ...def.includes].map(i => `attribute_list<"${i}">`).join(", ")}`
        }
        attributeListSection += " { ";
        indentLevel++;
        const x = defs.flatMap(d => d.attributes);
        if (def.attributes.length > 0) attributeListSection += "\n" + def.attributes.map(attr => indent(`attribute<"${attr.attrName}", ${makeAttrVariant(x.find(b => attr.attrName === b.attrName).attrType)}> ${attr.attrName};\n`)).join("");
        indentLevel--;
        attributeListSection += "};\n\n"
    })
    return attributeListSection;
}

function makeInterfaceSpecSection(defs: IdlInterface[]) {
    let interfaceSpecSection = "";
    interfaceSpecSection += indent("namespace interface {\n");
    indentLevel++;
    defs.forEach(def => interfaceSpecSection += indent(`using ${def.interfaceName} = interface_spec<"${def.interfaceName}", std::tuple<${[...def.parents, ...def.includes].join(", ")}>${def.attributes.length > 0 ? ", " + def.attributes.map(a => `&attribute_list<"${def.interfaceName}">::${a.attrName}`).join(", ") : ""}>;\n`));
    indentLevel--;
    interfaceSpecSection += indent("}\n");
    return interfaceSpecSection
}

function makeElementSection(defs: IdlInterface[]) {
    let elementSection = "";
    let count = 0;
    defs.forEach(def => {
        const tags = htmlElements.filter(e => e.interface === def.interfaceName).map(e => e.name);
        tags.forEach(tag => {
            count++;
            elementSection += indent(`template <class Cs> struct ${tag}: public element<"${tag}", ::ht::interface::${def.interfaceName}, Cs> {\n`);
            indentLevel++;
            elementSection += indent(`using element<"${tag}", ::ht::interface::${def.interfaceName}, Cs>::element;\n`);
            indentLevel--;
            elementSection += "};\n"
            elementSection += `template <class ...T> ${tag}(T&& ...t) -> ${tag}<typename ::ht::detail::child_types<T...>>;\n\n`
        });
    });
    console.log(`found ${count} elements`)
    return elementSection;
}

let interfaceNames = new Set<string>();
htmlElements.forEach(e => {
    getDependencies(e.interface).forEach(d => interfaceNames.add(d));
    interfaceNames.add(e.interface)
});

const defs = [...interfaceNames].map(i => {
    let idlInterface: IdlInterface = {
        interfaceName: i,
        parents: [htmlIdl.idlNames[i], ...(htmlIdl.idlExtendedNames[i] || [])].map(ext => ext?.inheritance).filter(i => i),
        includes: [htmlIdl.idlNames[i], ...(htmlIdl.idlExtendedNames[i] || []).filter(ext => ext && ext["type"] === "includes")].flatMap(ext => ext?.includes).filter(i => i),
        attributes: [htmlIdl.idlNames[i], ...(htmlIdl.idlExtendedNames[i] || [])].flatMap(ext => ext?.members).filter(m => m && m["type"] === "attribute").map(attr => ({
            attrName: attr["name"],
            attrType: makeAttributeType(attr["idlType"]["idlType"])
        }))
    };
    return idlInterface;
}).filter(d => !skipInterfaces.has(d.interfaceName));

console.log(`found ${defs.length} interfaces`);
writeFile("out/attribute_spec.def", makeAttributeSpecSection(defs), () => { console.log("wrote attribute_spec.def") });
writeFile("out/attribute_lists.def", makeAttributeListSection(defs), () => { console.log("wrote attribute_lists.def") });
writeFile("out/interface_spec.def", makeInterfaceSpecSection(defs), () => { console.log("wrote interface_spec.def") });
writeFile("out/elements.def", makeElementSection(defs), () => { console.log("wrote elements.def") });

