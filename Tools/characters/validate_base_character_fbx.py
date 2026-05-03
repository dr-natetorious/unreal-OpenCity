#!/usr/bin/env python3
"""
Validate the exported shared base-character FBX against the OpenCity contract.

Run:
  blender --background --factory-startup --python Tools/characters/validate_base_character_fbx.py -- \
    --fbx /apps/git/unreal-OpenCity/Content/Props/Characters/SK_CityCharacter.fbx
"""

import argparse
import json
import os
import sys

import bpy


def fail(msg):
    print(f"[FAIL] {msg}")


def ok(msg):
    print(f"[OK] {msg}")


def parse_args():
    argv = sys.argv
    if "--" in argv:
        argv = argv[argv.index("--") + 1 :]
    else:
        argv = []

    parser = argparse.ArgumentParser()
    parser.add_argument("--fbx", required=True)
    parser.add_argument(
        "--contract",
        default="/apps/git/unreal-OpenCity/Tools/characters/standard_contract.json",
    )
    return parser.parse_args(argv)


def load_contract(path):
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


def reset_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)


def find_armature(contract):
    armature_name = contract["base_character"].get("armature_name")
    if armature_name:
        arm = bpy.data.objects.get(armature_name)
        if arm and arm.type == "ARMATURE":
            return arm

    arms = [o for o in bpy.data.objects if o.type == "ARMATURE"]
    return arms[0] if arms else None


def find_mesh(contract):
    mesh_name = contract["base_character"].get("primary_mesh_object")
    if mesh_name:
        mesh = bpy.data.objects.get(mesh_name)
        if mesh and mesh.type == "MESH":
            return mesh

    meshes = [o for o in bpy.data.objects if o.type == "MESH"]
    return meshes[0] if meshes else None


def validate_fbx(contract):
    errors = []
    arm = find_armature(contract)
    if arm is None:
        return ["No armature found after FBX import"]

    required_bones = contract["base_character"].get("required_helper_bones", [])
    found_bones = [b.name for b in arm.data.bones]
    missing_bones = [b for b in required_bones if b not in found_bones]
    if missing_bones:
        errors.append(f"Exported FBX missing helper bones: {missing_bones}")
    else:
        ok(f"Exported FBX contains helper bones: {required_bones}")

    mesh = find_mesh(contract)
    if mesh is None:
        return errors + ["No mesh found after FBX import"]

    if mesh.parent != arm:
        parent_name = mesh.parent.name if mesh.parent else "None"
        errors.append(f"Exported mesh parent mismatch: expected {arm.name}, found {parent_name}")

    slot_names = [slot.material.name if slot.material else "" for slot in mesh.material_slots]
    required_slots = contract["base_character"].get("required_material_slots", [])
    missing_slots = [slot for slot in required_slots if slot not in slot_names]
    if missing_slots:
        errors.append(f"Exported FBX missing material slots: {missing_slots}")
    else:
        ok(f"Exported FBX contains material slots: {required_slots}")

    return errors


def main():
    args = parse_args()
    if not os.path.exists(args.contract):
        print(f"Contract file not found: {args.contract}")
        sys.exit(2)
    if not os.path.exists(args.fbx):
        print(f"FBX file not found: {args.fbx}")
        sys.exit(2)

    contract = load_contract(args.contract)
    reset_scene()
    bpy.ops.import_scene.fbx(filepath=args.fbx)

    errors = validate_fbx(contract)
    if errors:
        print("\nHARD BLOCK: exported FBX does not meet the base-character contract")
        for error in errors:
            fail(error)
        sys.exit(1)

    print("\nValidation passed")
    sys.exit(0)


if __name__ == "__main__":
    main()