#!/usr/bin/env python3
"""
Validate a character .blend against OpenCity character standards.

Run:
  blender --background /path/to/File.blend --python Tools/characters/validate_character_blend.py -- --role firefighter
  blender --background /path/to/File.blend --python Tools/characters/validate_character_blend.py -- --role police
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
    parser.add_argument("--role", choices=["base", "firefighter", "police"], required=True)
    parser.add_argument(
        "--contract",
        default="/apps/git/unreal-OpenCity/Tools/characters/standard_contract.json",
    )
    return parser.parse_args(argv)


def load_contract(path):
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


def find_primary_armature(role):
    arms = [o for o in bpy.data.objects if o.type == "ARMATURE"]
    if not arms:
        return None

    if role == "base":
        for arm in arms:
            if arm.name == "SK_Armature":
                return arm
        return arms[0]

    wanted = "ARM_Firefighter" if role == "firefighter" else "ARM_Police"
    for arm in arms:
        if arm.name == wanted:
            return arm

    role_key = "fire" if role == "firefighter" else "police"
    for arm in arms:
        if role_key in arm.name.lower():
            return arm

    return arms[0]


def find_primary_mesh(contract):
    base_contract = contract.get("base_character", {})
    mesh_name = base_contract.get("primary_mesh_object")
    if mesh_name:
        mesh = bpy.data.objects.get(mesh_name)
        if mesh and mesh.type == "MESH":
            return mesh

    meshes = [o for o in bpy.data.objects if o.type == "MESH"]
    return meshes[0] if meshes else None


def validate_base_character(contract, arm):
    errors = []
    base_contract = contract.get("base_character", {})

    required_bones = base_contract.get("required_helper_bones", [])
    found_bones = [b.name for b in arm.data.bones]
    missing_bones = [b for b in required_bones if b not in found_bones]
    if missing_bones:
        errors.append(f"Missing helper bones: {missing_bones}")
    else:
        ok(f"Helper bones present: {required_bones}")

    mesh = find_primary_mesh(contract)
    if mesh is None:
        errors.append("No mesh found for base character validation")
        return errors

    if mesh.parent != arm:
        errors.append(f"{mesh.name}: expected parent {arm.name}, found {mesh.parent.name if mesh.parent else 'None'}")

    slot_names = [slot.material.name if slot.material else "" for slot in mesh.material_slots]
    required_slots = base_contract.get("required_material_slots", [])
    missing_slots = [slot for slot in required_slots if slot not in slot_names]
    if missing_slots:
        errors.append(f"Missing material slots on {mesh.name}: {missing_slots}")
    else:
        ok(f"Material slots present on {mesh.name}: {required_slots}")

    return errors


def validate(role, contract):
    errors = []

    arm = find_primary_armature(role)
    if arm is None:
        errors.append("No armature found")
        return errors

    if role == "base":
        return validate_base_character(contract, arm)

    required_bones = contract["skeleton"]["required_bones"]
    found_bones = [b.name for b in arm.data.bones]

    missing = [b for b in required_bones if b not in found_bones]
    if missing:
        errors.append(f"Missing required bones: {missing}")
    else:
        ok("Required bones present")

    expected_count = contract["skeleton"].get("required_bone_count", 0)
    if expected_count and len(found_bones) != expected_count:
        errors.append(
            f"Bone count mismatch: found {len(found_bones)} expected {expected_count}"
        )
    else:
        ok(f"Bone count: {len(found_bones)}")

    if contract["skeleton"].get("armature_at_origin", False):
        loc = tuple(round(v, 6) for v in arm.location)
        if loc != (0.0, 0.0, 0.0):
            errors.append(f"Armature is not at origin: {loc}")
        else:
            ok("Armature at origin")

    # Validate role actions exist
    required_actions = contract["actions"].get(role, [])
    action_names = [a.name for a in bpy.data.actions]
    missing_actions = [a for a in required_actions if a not in action_names]
    if missing_actions:
        errors.append(f"Missing required actions: {missing_actions}")
    else:
        ok(f"Required actions present: {required_actions}")

    # Validate mesh binding for meshes parented to this armature
    meshes = [o for o in bpy.data.objects if o.type == "MESH" and o.parent == arm]
    if not meshes:
        errors.append("No meshes parented to primary armature")
    else:
        ok(f"Meshes parented to armature: {len(meshes)}")

    for m in meshes:
        if contract["binding"].get("allow_bone_parenting_as_final", False) is False:
            if m.parent_type == "BONE":
                errors.append(f"{m.name}: parent_type is BONE (not allowed)")

        if contract["binding"].get("require_armature_modifier", True):
            has_arm_mod = any(mod.type == "ARMATURE" and mod.object == arm for mod in m.modifiers)
            if not has_arm_mod:
                errors.append(f"{m.name}: missing Armature modifier targeting {arm.name}")

        if contract["binding"].get("require_vertex_groups", True):
            if len(m.vertex_groups) == 0:
                errors.append(f"{m.name}: no vertex groups")

    return errors


def main():
    args = parse_args()

    if not os.path.exists(args.contract):
        print(f"Contract file not found: {args.contract}")
        sys.exit(2)

    contract = load_contract(args.contract)
    errors = validate(args.role, contract)

    if errors:
        print("\nValidation errors:")
        for e in errors:
            fail(e)
        sys.exit(1)

    print("\nValidation passed")
    sys.exit(0)


if __name__ == "__main__":
    main()
