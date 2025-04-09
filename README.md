# Predictors
Branch Predictors

We can provide a rough performance comparison chart based on the data from public research. Usually, the performance of branch predictors is measured by Correct Prediction Rate (CPR) or Misprediction Per Kilo-Instructions (MPKI), where lower MPKI indicates better prediction performance.

At the same time, we provide some predictor code test cases on gem 5, gem 5 URL: https://www.gem5.org/

In general, TAGE-SC-L and multi-perspective perceptron predictors outperform Gshare and Agree predictors, mainly because they can more effectively use historical information to reduce misprediction rates.
