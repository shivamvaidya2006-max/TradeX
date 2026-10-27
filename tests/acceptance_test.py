import urllib.request
import json

BASE = "http://localhost:8080/api/v1"

def post(url, payload, token=None):
    req = urllib.request.Request(
        BASE + url,
        data=json.dumps(payload).encode('utf-8'),
        headers={'Content-Type': 'application/json', **({'Authorization': 'Bearer ' + token} if token else {})}
    )
    with urllib.request.urlopen(req) as resp:
        return json.loads(resp.read().decode('utf-8'))

def get(url, token=None):
    req = urllib.request.Request(
        BASE + url,
        headers={'Content-Type': 'application/json', **({'Authorization': 'Bearer ' + token} if token else {})}
    )
    with urllib.request.urlopen(req) as resp:
        return json.loads(resp.read().decode('utf-8'))

print("==============================================")
print("     TRADEX END-TO-END ACCEPTANCE TEST        ")
print("==============================================")

# 1. Login
login_res = post("/auth/login", {"username": "trader", "password": "trader123"})
assert login_res["success"], "Login failed"
token = login_res["data"]["token"]
print("[PASS] 1. User Authenticated with JWT:", login_res["data"]["user"]["username"])

# 2. Stock Search with Trie
search_res = get("/stocks/search?q=TCS", token)
assert search_res["success"], "Search failed"
assert len(search_res["data"]) > 0, "No suggestions returned"
print(f"[PASS] 2. Trie Autocomplete Search returned {len(search_res['data'])} suggestions for 'TCS'")

# 3. Portfolio before trade
port_before = get("/portfolio", token)["data"]
initial_balance = port_before["wallet"]["availableBalance"]
initial_tcs_qty = next((h["quantity"] for h in port_before["holdings"] if h["symbol"] == "TCS"), 0)
print(f"[PASS] 3. Portfolio fetched: Available Funds = INR{initial_balance:.2f}, Existing TCS = {initial_tcs_qty}")

# 4. Place BUY order for 10 shares of TCS
buy_req = {
    "symbol": "TCS",
    "transactionType": "BUY",
    "orderType": "MARKET",
    "quantity": 10,
    "price": 0.0
}
buy_res = post("/orders", buy_req, token)
assert buy_res["success"], f"Buy order failed: {buy_res.get('message')}"
print(f"[PASS] 4. BUY Order Executed: OrderID #{buy_res['data']['orderId']}, Executed Price: INR{buy_res['data']['executionPrice']:.2f}")

# 5. Verify Portfolio after BUY
port_after_buy = get("/portfolio", token)["data"]
new_balance = port_after_buy["wallet"]["availableBalance"]
assert new_balance < initial_balance, "Wallet balance did not decrease after BUY"
tcs_holding = next((h for h in port_after_buy["holdings"] if h["symbol"] == "TCS"), None)
assert tcs_holding is not None, "TCS holding not found after BUY"
assert tcs_holding["quantity"] == initial_tcs_qty + 10, f"Expected {initial_tcs_qty + 10} TCS shares, got {tcs_holding['quantity']}"
print(f"[PASS] 5. Wallet updated: INR{new_balance:.2f}, Holdings updated: {tcs_holding['quantity']} shares of TCS")

# 6. Place SELL order for 5 shares of TCS
sell_req = {
    "symbol": "TCS",
    "transactionType": "SELL",
    "orderType": "MARKET",
    "quantity": 5,
    "price": 0.0
}
sell_res = post("/orders", sell_req, token)
assert sell_res["success"], f"Sell order failed: {sell_res.get('message')}"
print(f"[PASS] 6. SELL Order Executed: OrderID #{sell_res['data']['orderId']}, Net Proceeds credited")

# 7. Verify Holdings after SELL
port_after_sell = get("/portfolio", token)["data"]
tcs_holding_after_sell = next((h for h in port_after_sell["holdings"] if h["symbol"] == "TCS"), None)
assert tcs_holding_after_sell["quantity"] == initial_tcs_qty + 5, f"Expected {initial_tcs_qty + 5} TCS shares, got {tcs_holding_after_sell['quantity']}"
print(f"[PASS] 7. Remaining Holdings: {tcs_holding_after_sell['quantity']} shares of TCS")

# 8. Test DSA Lab Endpoints
# 8a. Trie
trie_data = get("/dsa/trie/visualize?q=TA")
assert trie_data["success"], "Trie visualize failed"
print(f"[PASS] 8a. DSA Trie Visualization: Root character '{trie_data['data']['tree']['character']}'")

# 8b. OrderBook Heap
ob_data = get("/dsa/orderbook/TCS")
assert ob_data["success"], "OrderBook heap failed"
print(f"[PASS] 8b. DSA Heap OrderBook: Symbol {ob_data['data']['symbol']}, LTP INR{ob_data['data']['ltp']}")

# 8c. BST
bst_data = get("/dsa/bst/tree")
assert bst_data["success"], "BST tree failed"
print(f"[PASS] 8c. DSA BST Tree: Inorder traversal has {len(bst_data['data']['inorder'])} price levels")

# 8d. Graph BFS & DFS
graph_bfs = get("/dsa/graph/bfs?start=Technology")
graph_dfs = get("/dsa/graph/dfs?start=Technology")
assert graph_bfs["success"] and graph_dfs["success"], "Graph traversals failed"
print(f"[PASS] 8d. DSA Graph: BFS visited {len(graph_bfs['data']['traversalOrder'])} nodes, DFS visited {len(graph_dfs['data']['traversalOrder'])} nodes")

# 8e. Sorting Visualizer
sort_data = post("/dsa/sort", {"algorithm": "Quick Sort", "array": [4210.8, 1885.25, 2945.5, 788.9, 1642.4]})
assert sort_data["success"], "Sorting failed"
print(f"[PASS] 8e. DSA Quick Sort: Sorted in {sort_data['data']['durationMicroseconds']:.2f} µs ({sort_data['data']['comparisons']} comparisons, {sort_data['data']['swaps']} swaps)")

# 8f. Searching Visualizer
search_data = post("/dsa/search", {"algorithm": "Binary Search", "target": 1885.25, "array": [788.9, 1642.4, 1885.25, 2945.5, 4210.8]})
assert search_data["success"], "Search failed"
assert search_data["data"]["foundIndex"] == 2, "Binary search failed to find target"
print(f"[PASS] 8f. DSA Binary Search: Found index {search_data['data']['foundIndex']} in {search_data['data']['comparisons']} comparisons (O(log N))")

# 8g. Sliding Window Analytics
sw_data = get("/dsa/analytics/sliding-window?symbol=TCS&window=5")
assert sw_data["success"], "Sliding window failed"
print(f"[PASS] 8g. DSA Sliding Window & Two-Pointer: Best 5-day gain INR{sw_data['data']['bestWindow']['absoluteGain']:.2f} ({sw_data['data']['bestWindow']['percentageGain']:.2f}%)")

# 8h. Greedy Portfolio Allocation
greedy_data = post("/dsa/greedy/allocate", {"capital": 100000.0})
assert greedy_data["success"], "Greedy allocation failed"
print(f"[PASS] 8h. DSA Greedy Allocation: Allocated INR{greedy_data['data']['capitalAllocated']:.2f} across {len(greedy_data['data']['allocations'])} stocks")

# 8i. DP Max Profit
dp_data = get("/dsa/dp/max-profit?symbol=TCS")
assert dp_data["success"], "DP profit failed"
print(f"[PASS] 8i. DSA Dynamic Programming: Single Tx Max Profit INR{dp_data['data']['singleTransaction']['maxProfit']:.2f}, Two Tx Max Profit INR{dp_data['data']['twoTransactionsDP']['maxProfit']:.2f}")

# 8j. Audit Trail (Doubly Linked List)
audit_data = get("/dsa/audit-trail")
assert audit_data["success"], "Audit trail failed"
print(f"[PASS] 8j. DSA Doubly Linked List Audit Trail: {len(audit_data['data']['auditTrail'])} events recorded")

print("==============================================")
print("     ALL ACCEPTANCE CRITERIA PASSED!          ")
print("==============================================")
