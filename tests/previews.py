"""Validate the offline preview bundle. Requires Pillow; run from any directory."""
from pathlib import Path
import hashlib
import json
from PIL import Image

root = Path(__file__).resolve().parents[1]
catalog = json.loads((root / 'data/catalog.json').read_text())
extras = json.loads((root / 'data/extras.json').read_text())
manifest = json.loads((root / 'data/previews.json').read_text())
expected = {(row['definition'], row['paint']) for row in catalog['finishes']}
expected.update((row['id'], 0) for row in extras['agents'])
actual = {(row['definition'], row['paint']) for row in manifest['entries']}
assert expected == actual
assert len(manifest['entries']) == len(actual)
for row in manifest['entries']:
    assert row['file'] == f"{row['definition']}_{row['paint']}.png"
    path = root / 'bin/previews' / row['file']
    assert hashlib.sha256(path.read_bytes()).hexdigest() == row['sha256']
    with Image.open(path) as image:
        assert image.format == 'PNG'
        assert 0 < image.width <= 320 and 0 < image.height <= 224
        image.load()
assert len(list((root / 'bin/previews').glob('*.png'))) == len(actual)
print(f'{len(actual)} offline previews: coverage, hashes and decoding passed')
