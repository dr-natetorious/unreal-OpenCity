"""
OpenCity content bootstrap — run once from the editor Output Log.

Switch the Cmd dropdown to Python, then:
    exec(open('/apps/git/unreal-OpenCity/Tools/setup_content.py').read())

Creates:
  /Game/Input/   IA_Foot_Move, IA_Foot_Look, IA_Foot_Jump, IMC_Foot
  /Game/Maps/    TestLevel  (floor, sun, sky, player start)
"""

import unreal

at   = unreal.AssetToolsHelpers.get_asset_tools()
eal  = unreal.EditorAssetLibrary
lvl  = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
aas  = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

# ── helpers ────────────────────────────────────────────────────────────────────

def ensure_dir(path):
    if not eal.does_directory_exist(path):
        eal.make_directory(path)

def get_or_create(name, pkg, cls, factory_cls):
    full = f'{pkg}/{name}'
    if eal.does_asset_exist(full):
        return unreal.load_asset(full)
    return at.create_asset(name, pkg, cls, factory_cls())

def make_key(name):
    k = unreal.Key()
    k.set_editor_property('key_name', name)
    return k

def make_mapping(imc, action, key_name, mods=None):
    m = unreal.EnhancedActionKeyMapping()
    m.set_editor_property('action', action)
    m.set_editor_property('key', make_key(key_name))
    if mods:
        m.set_editor_property('modifiers', mods)
    return m

def negate(imc):
    return unreal.new_object(unreal.InputModifierNegate, imc)

def swizzle_yxz(imc):
    m = unreal.new_object(unreal.InputModifierSwizzleAxis, imc)
    m.set_editor_property('order', unreal.InputAxisSwizzle.YXZ)
    return m

# ── Input Actions ──────────────────────────────────────────────────────────────

ensure_dir('/Game/Input')

ia_move = get_or_create('IA_Foot_Move', '/Game/Input', unreal.InputAction, unreal.InputAction_Factory)
ia_move.set_editor_property('value_type', unreal.InputActionValueType.AXIS2D)
eal.save_asset('/Game/Input/IA_Foot_Move')

ia_look = get_or_create('IA_Foot_Look', '/Game/Input', unreal.InputAction, unreal.InputAction_Factory)
ia_look.set_editor_property('value_type', unreal.InputActionValueType.AXIS2D)
eal.save_asset('/Game/Input/IA_Foot_Look')

ia_jump = get_or_create('IA_Foot_Jump', '/Game/Input', unreal.InputAction, unreal.InputAction_Factory)
ia_jump.set_editor_property('value_type', unreal.InputActionValueType.AXIS1D)
eal.save_asset('/Game/Input/IA_Foot_Jump')

print('[setup] Input actions: IA_Foot_Move (Axis2D), IA_Foot_Look (Axis2D), IA_Foot_Jump (Axis1D)')

# ── Shared interact action ─────────────────────────────────────────────────────

ia_interact = get_or_create('IA_Interact', '/Game/Input', unreal.InputAction, unreal.InputAction_Factory)
ia_interact.set_editor_property('value_type', unreal.InputActionValueType.BOOLEAN)
eal.save_asset('/Game/Input/IA_Interact')

# ── Vehicle input actions ──────────────────────────────────────────────────────

ia_throttle = get_or_create('IA_Vehicle_Throttle', '/Game/Input', unreal.InputAction, unreal.InputAction_Factory)
ia_throttle.set_editor_property('value_type', unreal.InputActionValueType.AXIS1D)
eal.save_asset('/Game/Input/IA_Vehicle_Throttle')

ia_steer = get_or_create('IA_Vehicle_Steer', '/Game/Input', unreal.InputAction, unreal.InputAction_Factory)
ia_steer.set_editor_property('value_type', unreal.InputActionValueType.AXIS1D)
eal.save_asset('/Game/Input/IA_Vehicle_Steer')

print('[setup] Vehicle actions: IA_Interact (Boolean), IA_Vehicle_Throttle (Axis1D), IA_Vehicle_Steer (Axis1D)')

# ── Input Mapping Context ──────────────────────────────────────────────────────

imc = get_or_create('IMC_Foot', '/Game/Input', unreal.InputMappingContext, unreal.InputMappingContext_Factory)

# Move — Axis2D: X = right/left, Y = forward/back
#   W → +Y:  swizzle(YXZ) turns raw (1,0,0) → (0,1,0)
#   S → -Y:  negate then swizzle
#   D → +X:  raw (1,0,0) already correct, no modifier
#   A → -X:  negate
imc.set_editor_property('mappings', [
    make_mapping(imc, ia_move,     'W',         [swizzle_yxz(imc)]),
    make_mapping(imc, ia_move,     'S',         [negate(imc), swizzle_yxz(imc)]),
    make_mapping(imc, ia_move,     'D'),
    make_mapping(imc, ia_move,     'A',         [negate(imc)]),
    make_mapping(imc, ia_look,     'Mouse2D'),
    make_mapping(imc, ia_jump,     'SpaceBar'),
    make_mapping(imc, ia_interact, 'E'),
])

eal.save_asset('/Game/Input/IMC_Foot')
print('[setup] IMC_Foot: WASD move, Mouse2D look, Space jump, E interact')

# ── Vehicle Mapping Context ────────────────────────────────────────────────────

imc_v = get_or_create('IMC_Vehicle', '/Game/Input', unreal.InputMappingContext, unreal.InputMappingContext_Factory)
imc_v.set_editor_property('mappings', [
    make_mapping(imc_v, ia_throttle, 'W'),
    make_mapping(imc_v, ia_throttle, 'S', [negate(imc_v)]),
    make_mapping(imc_v, ia_steer,    'D'),
    make_mapping(imc_v, ia_steer,    'A', [negate(imc_v)]),
    make_mapping(imc_v, ia_interact, 'E'),
])

eal.save_asset('/Game/Input/IMC_Vehicle')
print('[setup] IMC_Vehicle: W/S throttle, A/D steer, E exit')

# ── Level actors ───────────────────────────────────────────────────────────────
# The static floor and city blocks are gone — UCityStreamingSubsystem handles
# both at runtime, tiling infinitely around the player.
# This section only places actors that must be in the level persistently.

# PlayerStart — ensure it's above Z=0 so the character spawns above the first floor tile
for actor in aas.get_all_level_actors():
    if actor.get_class().get_name() == 'PlayerStart':
        actor.set_actor_location(unreal.Vector(0, 0, 100), False, False)
        break

# Car — one near spawn so there's always something to drive
car_class = unreal.load_class(None, '/Script/OpenCity.CarPawn')
if car_class:
    existing_cars = [a for a in aas.get_all_level_actors() if a.get_class() == car_class]
    if not existing_cars:
        car = aas.spawn_actor_from_class(car_class, unreal.Vector(300, 0, 100))
        car.set_actor_label('PlayerCar')
        print('[setup] Car: spawned at (300, 0, 100)')
    else:
        print(f'[setup] Car: {len(existing_cars)} already in level — skipped')
else:
    print('[setup] WARNING: Could not load CarPawn class')

lvl.save_current_level()
print('[setup] Level saved')
print()
print('Press Play — city streams in automatically around the player.')
print('WASD = move  |  Mouse = look  |  Space = jump  |  E = enter/exit car')

# ── setup_level(label) ─────────────────────────────────────────────────────────
# Adds sun, sky, fog, a floor plane, and a PlayerStart to the current map.
# Call this after opening any new map that needs basic lighting.
#
# Usage (UE Output Log, Python mode):
#   exec(open('/apps/git/unreal-OpenCity/Tools/setup_content.py').read())
#   setup_level('GASTest')

def setup_level(label='Level'):
    actors = aas.get_all_level_actors()
    class_names = {a.get_class().get_name() for a in actors}

    def has(cls_name):
        return cls_name in class_names

    def spawn_if_missing(cls_path, actor_label, location=unreal.Vector(0,0,0)):
        cls = unreal.load_class(None, cls_path)
        if not cls:
            print(f'[setup_level] WARNING: could not load {cls_path}')
            return None
        actor = aas.spawn_actor_from_class(cls, location)
        if actor:
            actor.set_actor_label(actor_label)
            print(f'[setup_level] Spawned {actor_label}')
        return actor

    # Directional light (sun) — 45-degree pitch so shadows are visible
    if not has('DirectionalLight'):
        sun = spawn_if_missing('/Script/Engine.DirectionalLight', 'SunLight',
                               unreal.Vector(0, 0, 300))
        if sun:
            sun.set_actor_rotation(unreal.Rotator(-45, -60, 0), False)
            comp = sun.get_component_by_class(unreal.DirectionalLightComponent)
            if comp:
                comp.set_editor_property('intensity', 10.0)
                comp.set_editor_property('light_color', unreal.Color(255, 247, 225, 255))
                comp.set_editor_property('atmosphere_sun_light', True)
    else:
        print('[setup_level] DirectionalLight: already present — skipped')

    # Sky atmosphere
    if not has('SkyAtmosphere'):
        spawn_if_missing('/Script/Engine.SkyAtmosphere', 'SkyAtmosphere')
    else:
        print('[setup_level] SkyAtmosphere: already present — skipped')

    # Sky light (real-time capture)
    if not has('SkyLight'):
        sky = spawn_if_missing('/Script/Engine.SkyLight', 'SkyLight')
        if sky:
            comp = sky.get_component_by_class(unreal.SkyLightComponent)
            if comp:
                comp.set_editor_property('real_time_capture', True)
    else:
        print('[setup_level] SkyLight: already present — skipped')

    # Exponential height fog
    if not has('ExponentialHeightFog'):
        fog = spawn_if_missing('/Script/Engine.ExponentialHeightFog', 'HeightFog',
                               unreal.Vector(0, 0, 100))
        if fog:
            comp = fog.get_component_by_class(unreal.ExponentialHeightFogComponent)
            if comp:
                comp.set_editor_property('fog_density', 0.02)
    else:
        print('[setup_level] ExponentialHeightFog: already present — skipped')

    # Floor plane — 10,000 x 10,000 cm (100 m) flat surface
    if not any(a.get_actor_label() == 'TestFloor' for a in actors):
        floor_mesh = unreal.load_object(None, '/Engine/BasicShapes/Plane.Plane')
        floor_cls  = unreal.load_class(None, '/Script/Engine.StaticMeshActor')
        if floor_cls and floor_mesh:
            floor = aas.spawn_actor_from_class(floor_cls, unreal.Vector(0, 0, 0))
            if floor:
                floor.set_actor_label('TestFloor')
                floor.set_actor_scale3d(unreal.Vector(100, 100, 1))
                mesh_comp = floor.get_component_by_class(unreal.StaticMeshComponent)
                if mesh_comp:
                    mesh_comp.set_static_mesh(floor_mesh)
                print('[setup_level] TestFloor: 100x100m plane at origin')
    else:
        print('[setup_level] TestFloor: already present — skipped')

    # PlayerStart — move to Z=100 so character spawns above the floor
    for actor in aas.get_all_level_actors():
        if actor.get_class().get_name() == 'PlayerStart':
            actor.set_actor_location(unreal.Vector(0, 0, 100), False, False)
            print('[setup_level] PlayerStart: moved to (0,0,100)')
            break
    else:
        ps_cls = unreal.load_class(None, '/Script/Engine.PlayerStart')
        if ps_cls:
            ps = aas.spawn_actor_from_class(ps_cls, unreal.Vector(0, 0, 100))
            if ps:
                ps.set_actor_label('PlayerStart')
                print('[setup_level] PlayerStart: spawned at (0,0,100)')

    lvl.save_current_level()
    print(f'[setup_level] {label}: lighting + floor setup complete. Level saved.')
