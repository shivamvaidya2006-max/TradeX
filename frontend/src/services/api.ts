import axios from 'axios';
import { 
  Stock, Watchlist, Order, Holding, Position, Wallet, 
  PortfolioSummary, Transaction, NotificationItem, 
  MarketOverview, PriceHistoryPoint, TrieVisualNode, 
  OrderBookData, BSTData, GraphNode, GraphEdge, 
  SortResult, SearchResult, SlidingWindowResult, 
  GreedyAllocationResult, DPProfitResult 
} from '../types';

const api = axios.create({
  baseURL: '/api/v1',
  headers: {
    'Content-Type': 'application/json',
  },
});

api.interceptors.request.use((config) => {
  const token = localStorage.getItem('tradex_token');
  if (token) {
    config.headers.Authorization = `Bearer ${token}`;
  }
  return config;
});

export const authApi = {
  login: (data: { username?: string; email?: string; password?: string }) => 
    api.post('/auth/login', data),
  register: (data: { fullName: string; email: string; phone: string; username: string; password: string }) => 
    api.post('/auth/register', data),
  getMe: () => api.get('/auth/me'),
};

export const stockApi = {
  getAll: () => api.get<{ data: Stock[] }>('/stocks'),
  getBySymbol: (symbol: string) => api.get<{ data: Stock }>(`/stocks/${symbol}`),
  search: (query: string) => api.get<{ data: any[] }>(`/stocks/search?q=${encodeURIComponent(query)}`),
  getHistory: (symbol: string, range: string = '1M') => 
    api.get<{ data: PriceHistoryPoint[] }>(`/stocks/${symbol}/history?range=${range}`),
  getMarketOverview: () => api.get<{ data: MarketOverview }>('/market/overview'),
  getMarketStatus: () => api.get('/market/status'),
  getOrderBook: (symbol: string) => api.get<{ data: OrderBookData }>(`/dsa/orderbook/${symbol}`),
};

export const watchlistApi = {
  getAll: () => api.get<{ data: Watchlist[] }>('/watchlists'),
  create: (name: string) => api.post('/watchlists', { name }),
  delete: (id: number) => api.delete(`/watchlists/${id}`),
  addItem: (watchlistId: number, stockId: number) => 
    api.post('/watchlists/items', { watchlistId, stockId }),
  removeItem: (watchlistId: number, stockId: number) => 
    api.delete('/watchlists/items', { data: { watchlistId, stockId } }),
};

export const orderApi = {
  placeOrder: (data: {
    symbol: string;
    transactionType: 'BUY' | 'SELL';
    orderType: 'MARKET' | 'LIMIT' | 'STOP_LOSS';
    quantity: number;
    price: number;
    triggerPrice?: number;
  }) => api.post('/orders', data),
  getAll: () => api.get<{ data: Order[] }>('/orders'),
  cancel: (orderId: number) => api.delete(`/orders/${orderId}`),
};

export const portfolioApi = {
  getPortfolio: () => api.get<{ data: {
    holdings: Holding[];
    positions: Position[];
    wallet: Wallet;
    summary: PortfolioSummary;
  } }>('/portfolio'),
};

export const walletApi = {
  getWallet: () => api.get<{ data: Wallet }>('/funds'),
  deposit: (amount: number) => api.post('/funds/deposit', { amount }),
  withdraw: (amount: number) => api.post('/funds/withdraw', { amount }),
  getTransactions: () => api.get<{ data: Transaction[] }>('/transactions'),
};

export const notificationApi = {
  getAll: () => api.get<{ data: NotificationItem[] }>('/notifications'),
};

export const dsaApi = {
  getTrieVisual: (prefix: string) => 
    api.get<{ data: { algorithm: string; tree: TrieVisualNode } }>(`/dsa/trie/visualize?q=${encodeURIComponent(prefix)}`),
  getOrderBook: (symbol: string) => 
    api.get<{ data: OrderBookData }>(`/dsa/orderbook/${symbol}`),
  getBST: () => 
    api.get<{ data: BSTData }>('/dsa/bst/tree'),
  insertBST: (price: number, symbol: string) => 
    api.post('/dsa/bst/insert', { price, symbol }),
  getGraphRelations: () => 
    api.get<{ data: { nodes: GraphNode[]; edges: GraphEdge[] } }>('/dsa/graph/relations'),
  runBFS: (start: string) => 
    api.get<{ data: { startNode: string; traversalOrder: string[] } }>(`/dsa/graph/bfs?start=${encodeURIComponent(start)}`),
  runDFS: (start: string) => 
    api.get<{ data: { startNode: string; traversalOrder: string[] } }>(`/dsa/graph/dfs?start=${encodeURIComponent(start)}`),
  runSort: (algorithm: string, array: number[]) => 
    api.post<{ data: SortResult }>('/dsa/sort', { algorithm, array }),
  runSearch: (algorithm: string, target: number, array: number[]) => 
    api.post<{ data: SearchResult }>('/dsa/search', { algorithm, target, array }),
  getSlidingWindow: (symbol: string, window: number = 5) => 
    api.get<{ data: SlidingWindowResult }>(`/dsa/analytics/sliding-window?symbol=${symbol}&window=${window}`),
  getGreedyAllocation: (capital: number) => 
    api.post<{ data: GreedyAllocationResult }>('/dsa/greedy/allocate', { capital }),
  getDPProfit: (symbol: string) => 
    api.get<{ data: DPProfitResult }>(`/dsa/dp/max-profit?symbol=${symbol}`),
  getAuditTrail: () => 
    api.get<{ data: { auditTrail: string[] } }>('/dsa/audit-trail'),
};

export const adminApi = {
  getUsers: () => api.get('/admin/users'),
  getSystemStats: () => api.get('/admin/system-stats'),
  startMarket: () => api.post('/admin/market/start'),
  stopMarket: () => api.post('/admin/market/stop'),
};

export default api;
