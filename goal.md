# Extending `mix` node to allow per channel mixing

The mix node's mix input should allow for all data formats as input and do a per element linear interpolation, not just float.

```
in1 = Vector2(1,1)
in2 = Vector2(0,0)
mix = Vector2(1,0)
out = Vector2(1,0)
```

Backwards compatibility means float needs to remain supported.

Adding node definitions with the extra suffix of the mix data type like `ND_mix_color3_color3`.


