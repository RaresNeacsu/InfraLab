# Azure Functions 2021 sample

`requests.json` contains 10,000 invocations from Microsoft's
[Azure Functions Invocation Trace 2021](https://github.com/Azure/AzurePublicDataset/blob/master/AzureFunctionsInvocationTrace2021.md).
The data is licensed under [CC BY 4.0](https://github.com/Azure/AzurePublicDataset/blob/master/LICENSE).

Attribution: Yanqi Zhang, Íñigo Goiri, Gohar Irfan Chaudhry, Rodrigo Fonseca,
Sameh Elnikety, Christina Delimitrou, Ricardo Bianchini.
*Faster and Cheaper Serverless Computing on Harvested Resources*, SOSP 2021.

## Conversion

- Read the source CSV using seconds, as specified in its dataset description.
- Exclude rows with nonpositive duration or nonfinite timing values.
- Sort by `end_timestamp - duration`, breaking ties by source CSV line number.
- Select the first 10,000 valid rows and subtract their first start time.
- Use this relative start as `arrival_time` and the observed duration as `work_units`.
- Assign sequential request IDs; source application and function IDs are omitted.

This is a duration replay: it does not reproduce the original Azure hardware,
queueing, cold starts, memory use or network. Source timestamps were modified by
Microsoft. The calculated execution start is used as the simulated arrival;
it is not a measured external HTTP arrival timestamp.

`metadata.json` records the source, checksums and sample statistics.
The last arrival is at about 10,764 seconds. Use a 12,000-second limit to allow draining.

## Full trace

The `requests/` directory contains all usable invocations from the source CSV,
split into parts of at most 50,000 requests. `requests_manifest.json` records the
counts, source time ranges and checksums. The exclusion and sorting rules above
apply to the full export too. No valid invocations are sampled out.

Each part is a standalone scenario with its first arrival at zero. IDs remain
unique across parts. Do not concatenate parts without restoring their source
time offsets from the manifest. These files describe the released trace, which
is not the whole Azure platform's traffic. `requests.json` remains the smaller
10,000-invocation sample and overlaps the beginning of the full export.

## Additional configurations

`servers/` contains 72 experimental configurations, listed in
`server_configurations.csv`: 3/6/12 servers, 2/4/8/16 workers per server,
and waiting limits of 0, 2 or 8 times the worker count. Reference servers all
use speed 1.0; mixed servers repeat relative speeds 0.75, 1.0 and 1.5.
These relative speeds are assumptions, not benchmark measurements.

For a conservative draining limit, use the part's last arrival plus
`max_duration * (1 + largest_waiting_limit) / smallest_speed`.

```sh
./build/oop_main --compare assets/azure_2021/servers/s3_w4_q2_reference.json assets/azure_2021/servers/s6_w4_q2_reference.json assets/azure_2021/requests/part_001.json
```

Use matching traffic and limits when comparing configurations. There is no
single best configuration independent of cost and response-time requirements.

## Server scenarios

Each file contains three servers. Each server has 2, 4 or 8 independent workers,
speed 1.0 per worker, and a waiting limit twice its worker count. These are
experimental configurations, not measured Azure machine specifications.

At speed 1.0, a request occupies one worker for its recorded duration. Increasing
worker count adds independent processing slots; it does not divide a fixed CPU budget.

From the project root, after building:

```sh
./build/oop_main --compare assets/azure_2021/servers_2_workers.json assets/azure_2021/servers_4_workers.json assets/azure_2021/requests.json
```

Enter `12000`. This command compares configurations using round robin. Final
reports are saved to `assets/comparison_report.txt`; states appear in the terminal.
With 10,000 requests, debug output is large. Redirect stdout to a temporary file
if needed. A different working directory requires corresponding asset paths.
