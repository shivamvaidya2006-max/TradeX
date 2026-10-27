# TradeX Trading & Financial Algorithms Specification

## 1. Continuous Double Auction Matching Engine

The core matching engine implements the **price-time priority** rule (FIFO within equal price levels) fundamental to modern electronic exchanges like NSE, BSE, NASDAQ, and CME.

```
Incoming Order (Buy or Sell)
          │
          ▼
   Order Validation (Funds, Holdings, Price Bounds)
          │
    ┌─────┴──────────────────────┐
    ▼                            ▼
Buy Order (Bid)            Sell Order (Ask)
    │                            │
    ▼                            ▼
Inspect Min-Heap Asks       Inspect Max-Heap Bids
If Bid >= Best Ask:         If Ask <= Best Bid:
    MATCH & EXECUTE TRADE       MATCH & EXECUTE TRADE
Else:                       Else:
    Push into Max-Heap Bids     Push into Min-Heap Asks
```

### Invariants:
1. **Spread Invariant:** $\text{Spread} = \text{Best Ask Price} - \text{Best Bid Price} > 0$. If this becomes $\le 0$, a match must immediately occur.
2. **Partial Fill:** If the incoming quantity exceeds the top resting order quantity, the top order is fully filled and popped, and the remaining quantity continues matching against subsequent heap elements until exhausted or the price cross condition ceases.

---

## 2. Geometric Brownian Motion Market Simulator

Stock prices do not move completely randomly; they exhibit drift ($\mu$) and diffusion ($\sigma$) following a **Geometric Brownian Motion (GBM)** stochastic differential equation:

$$dS_t = \mu S_t dt + \sigma S_t dW_t$$

Where:
- $S_t$: Stock price at time $t$.
- $\mu$: Drift coefficient (representing general macroeconomic growth or sectoral momentum).
- $\sigma$: Volatility parameter (individual stock beta).
- $W_t$: Standard Brownian motion (Wiener process), where $dW_t \sim \mathcal{N}(0, dt)$.

In discrete simulation time $\Delta t$, the price step is computed in C++ via:

$$S_{t + \Delta t} = S_t \exp\left( \left(\mu - \frac{\sigma^2}{2}\right) \Delta t + \sigma \sqrt{\Delta t} \, Z \right)$$

Where $Z \sim \mathcal{N}(0, 1)$ is drawn using C++ standard `<random>` `std::normal_distribution<double>`.

---

## 3. Configurable Regulatory & Brokerage Charges Engine

TradeX simulates the exact charge structure enforced by Indian brokerages and capital market regulators:

1. **Brokerage:**
   $$\text{Brokerage} = \min\left(20.00, \; \text{Turnover} \times 0.03\%\right)$$
2. **Securities Transaction Tax (STT):**
   $$\text{STT} = \begin{cases} \text{Turnover} \times 0.1\%, & \text{if Delivery (CNC)} \\ 0, & \text{if Intraday (MIS Buy)} \end{cases}$$
3. **Exchange Transaction Charges (NSE/BSE):**
   $$\text{Exchange Fee} = \text{Turnover} \times 0.00345\%$$
4. **Goods & Services Tax (GST):**
   $$\text{GST} = (\text{Brokerage} + \text{Exchange Fee}) \times 18\%$$
5. **SEBI Turnover Fee:**
   $$\text{SEBI Fee} = \text{Turnover} \times 0.0001\%$$
6. **Stamp Duty:**
   $$\text{Stamp Duty} = \begin{cases} \text{Turnover} \times 0.015\%, & \text{if BUY} \\ 0, & \text{if SELL} \end{cases}$$

---

## 4. Greedy Portfolio Capital Allocator (Sharpe-Heuristic)

Given total budget $C$ and a universe of $N$ equities, the algorithm solves an integer knapsack-like problem by ranking candidates according to a risk-adjusted return ratio:

$$\text{Efficiency Ratio } E_i = \frac{\mathbb{E}[R_i]}{\text{RiskScore}_i}$$

### Algorithm Steps:
1. Sort equities descending by $E_i$ in $O(N \log N)$ time.
2. Initialize remaining capital $C_{\text{rem}} = C$.
3. For each equity $i$:
   - Max share limit: $W_{\max} = 0.25 \times C$ (diversification cap).
   - Target allocation: $A_i = \min(C_{\text{rem}}, W_{\max})$.
   - Integer shares: $S_i = \lfloor A_i / P_i \rfloor$.
   - Deduct $S_i \times P_i$ from $C_{\text{rem}}$.
4. Return final share distributions, portfolio weights, and expected portfolio return.
