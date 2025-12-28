import React, { useState } from 'react'
import { Routes, Route, Link, useLocation } from 'react-router-dom'
import { useQuery } from '@tanstack/react-query'
import {
  LayoutDashboard,
  FolderTree,
  Layers,
  FileCode,
  AlertTriangle,
  CheckSquare,
  Search,
  Activity,
  Settings,
  RefreshCw,
  ChevronRight,
  ExternalLink
} from 'lucide-react'

// API helper
const api = async (endpoint) => {
  const res = await fetch(`/api${endpoint}`)
  if (!res.ok) throw new Error(`API error: ${res.status}`)
  return res.json()
}

// Sidebar Navigation
function Sidebar() {
  const location = useLocation()
  const navItems = [
    { path: '/', icon: LayoutDashboard, label: 'Overview' },
    { path: '/components', icon: FolderTree, label: 'Components' },
    { path: '/features', icon: Layers, label: 'Features' },
    { path: '/files', icon: FileCode, label: 'Files' },
    { path: '/stubs', icon: AlertTriangle, label: 'Stubs' },
    { path: '/tasks', icon: CheckSquare, label: 'Tasks' },
    { path: '/search', icon: Search, label: 'Search' },
  ]

  return (
    <aside className="w-64 bg-ombra-darker border-r border-gray-800 flex flex-col">
      <div className="p-4 border-b border-gray-800">
        <h1 className="text-xl font-bold text-ombra-accent">OMBRA</h1>
        <p className="text-xs text-gray-500">Project Management</p>
      </div>
      <nav className="flex-1 p-2">
        {navItems.map(({ path, icon: Icon, label }) => (
          <Link
            key={path}
            to={path}
            className={`flex items-center gap-3 px-3 py-2 rounded-lg mb-1 transition-colors ${
              location.pathname === path
                ? 'bg-ombra-accent/10 text-ombra-accent'
                : 'text-gray-400 hover:text-white hover:bg-gray-800'
            }`}
          >
            <Icon size={18} />
            <span>{label}</span>
          </Link>
        ))}
      </nav>
      <div className="p-4 border-t border-gray-800 text-xs text-gray-500">
        v1.0.0 | Port 1337
      </div>
    </aside>
  )
}

// Stat Card Component
function StatCard({ label, value, subValue, color = 'accent', icon: Icon }) {
  const colorClasses = {
    accent: 'text-ombra-accent border-ombra-accent/30',
    warning: 'text-ombra-warning border-ombra-warning/30',
    danger: 'text-ombra-danger border-ombra-danger/30',
  }
  return (
    <div className={`bg-ombra-darker border ${colorClasses[color]} rounded-lg p-4`}>
      <div className="flex items-center justify-between mb-2">
        <span className="text-gray-400 text-sm">{label}</span>
        {Icon && <Icon size={16} className={colorClasses[color].split(' ')[0]} />}
      </div>
      <div className={`text-2xl font-bold ${colorClasses[color].split(' ')[0]}`}>
        {value}
      </div>
      {subValue && <div className="text-xs text-gray-500 mt-1">{subValue}</div>}
    </div>
  )
}

// Progress Bar
function ProgressBar({ value, max, color = 'accent' }) {
  const percent = max > 0 ? (value / max * 100) : 0
  const colorClasses = {
    accent: 'bg-ombra-accent',
    warning: 'bg-ombra-warning',
    danger: 'bg-ombra-danger',
  }
  return (
    <div className="h-2 bg-gray-800 rounded-full overflow-hidden">
      <div
        className={`h-full ${colorClasses[color]} transition-all duration-500`}
        style={{ width: `${percent}%` }}
      />
    </div>
  )
}

// Dashboard Overview
function Dashboard() {
  const { data, isLoading, error, refetch } = useQuery({
    queryKey: ['dashboard'],
    queryFn: () => api('/dashboard'),
    refetchInterval: 30000,
  })

  if (isLoading) return <LoadingSpinner />
  if (error) return <ErrorMessage error={error} />

  const { overview, components, feature_breakdown, recent_activity, priority_tasks } = data

  return (
    <div className="space-y-6">
      {/* Header */}
      <div className="flex items-center justify-between">
        <div>
          <h2 className="text-2xl font-bold">Project Overview</h2>
          <p className="text-gray-500">Real-time codebase intelligence</p>
        </div>
        <button
          onClick={() => refetch()}
          className="flex items-center gap-2 px-4 py-2 bg-ombra-accent/10 text-ombra-accent rounded-lg hover:bg-ombra-accent/20 transition-colors"
        >
          <RefreshCw size={16} />
          Refresh
        </button>
      </div>

      {/* Stats Grid */}
      <div className="grid grid-cols-4 gap-4">
        <StatCard
          label="Features"
          value={overview.total_features}
          subValue={`${overview.implemented_features} implemented`}
          icon={Layers}
        />
        <StatCard
          label="Files Indexed"
          value={overview.total_files}
          subValue={`${overview.total_loc?.toLocaleString() || 0} LOC`}
          icon={FileCode}
        />
        <StatCard
          label="Stub Rate"
          value={`${overview.stub_percentage}%`}
          subValue={`${overview.stub_functions} of ${overview.total_functions}`}
          color={overview.stub_percentage > 20 ? 'danger' : overview.stub_percentage > 10 ? 'warning' : 'accent'}
          icon={AlertTriangle}
        />
        <StatCard
          label="Critical Stubs"
          value={overview.critical_stubs}
          subValue="In P0/P1 features"
          color={overview.critical_stubs > 0 ? 'danger' : 'accent'}
          icon={AlertTriangle}
        />
      </div>

      {/* Two Column Layout */}
      <div className="grid grid-cols-2 gap-6">
        {/* Components */}
        <div className="bg-ombra-darker border border-gray-800 rounded-lg p-4">
          <h3 className="text-lg font-semibold mb-4 flex items-center gap-2">
            <FolderTree size={18} className="text-ombra-accent" />
            Components
          </h3>
          <div className="space-y-3">
            {components?.length > 0 ? components.map((comp) => (
              <div key={comp.id} className="flex items-center justify-between p-3 bg-gray-800/50 rounded-lg">
                <div>
                  <div className="font-medium">{comp.name}</div>
                  <div className="text-xs text-gray-500">
                    {comp.module_count} modules | {comp.file_count} files
                  </div>
                </div>
                <div className="text-right">
                  <div className="text-sm text-gray-400">
                    {comp.total_loc?.toLocaleString() || 0} LOC
                  </div>
                </div>
              </div>
            )) : (
              <div className="text-gray-500 text-center py-4">
                No components registered. Run a codebase scan.
              </div>
            )}
          </div>
        </div>

        {/* Priority Tasks */}
        <div className="bg-ombra-darker border border-gray-800 rounded-lg p-4">
          <h3 className="text-lg font-semibold mb-4 flex items-center gap-2">
            <CheckSquare size={18} className="text-ombra-warning" />
            Priority Tasks
          </h3>
          <div className="space-y-2">
            {priority_tasks?.length > 0 ? priority_tasks.slice(0, 5).map((task) => (
              <div key={task.id} className="flex items-center gap-3 p-3 bg-gray-800/50 rounded-lg">
                <span className={`px-2 py-0.5 rounded text-xs priority-${task.priority}`}>
                  {task.priority}
                </span>
                <div className="flex-1 truncate">{task.title}</div>
                <span className="text-xs text-gray-500">{task.status}</span>
              </div>
            )) : (
              <div className="text-gray-500 text-center py-4">
                No priority tasks. Create some tasks to track.
              </div>
            )}
          </div>
        </div>
      </div>

      {/* Recent Activity */}
      <div className="bg-ombra-darker border border-gray-800 rounded-lg p-4">
        <h3 className="text-lg font-semibold mb-4 flex items-center gap-2">
          <Activity size={18} className="text-ombra-accent" />
          Recent Activity
        </h3>
        <div className="space-y-2">
          {recent_activity?.length > 0 ? recent_activity.slice(0, 8).map((activity, idx) => (
            <div key={idx} className="flex items-center gap-3 p-2 hover:bg-gray-800/30 rounded">
              <span className="text-xs text-gray-500 w-32">
                {new Date(activity.performed_at).toLocaleString()}
              </span>
              <span className="px-2 py-0.5 rounded text-xs bg-gray-700">
                {activity.action}
              </span>
              <span className="text-gray-400">{activity.entity_type}</span>
              <span className="text-white">{activity.entity_name || `#${activity.entity_id}`}</span>
            </div>
          )) : (
            <div className="text-gray-500 text-center py-4">
              No recent activity.
            </div>
          )}
        </div>
      </div>
    </div>
  )
}

// Components Page
function ComponentsPage() {
  const { data, isLoading, error } = useQuery({
    queryKey: ['components'],
    queryFn: () => api('/components'),
  })

  if (isLoading) return <LoadingSpinner />
  if (error) return <ErrorMessage error={error} />

  return (
    <div className="space-y-6">
      <div className="flex items-center justify-between">
        <h2 className="text-2xl font-bold">Components</h2>
      </div>
      <div className="grid gap-4">
        {data?.components?.map((comp) => (
          <div key={comp.id} className="bg-ombra-darker border border-gray-800 rounded-lg p-4">
            <div className="flex items-center justify-between mb-2">
              <h3 className="text-lg font-semibold">{comp.name}</h3>
              <span className="px-2 py-1 bg-gray-700 rounded text-xs">{comp.component_type}</span>
            </div>
            <p className="text-gray-400 text-sm mb-3">{comp.description}</p>
            <div className="flex gap-4 text-sm">
              <span className="text-gray-500">Path: <span className="text-gray-300">{comp.root_path}</span></span>
              <span className="text-gray-500">Language: <span className="text-gray-300">{comp.language}</span></span>
            </div>
          </div>
        ))}
        {(!data?.components || data.components.length === 0) && (
          <div className="text-center py-8 text-gray-500">
            No components registered. Use the MCP tools to register components.
          </div>
        )}
      </div>
    </div>
  )
}

// Features Page
function FeaturesPage() {
  const { data, isLoading, error } = useQuery({
    queryKey: ['features'],
    queryFn: () => api('/features'),
  })

  if (isLoading) return <LoadingSpinner />
  if (error) return <ErrorMessage error={error} />

  return (
    <div className="space-y-6">
      <h2 className="text-2xl font-bold">Features</h2>
      <div className="grid gap-4">
        {data?.features?.map((feat) => (
          <div key={feat.id} className="bg-ombra-darker border border-gray-800 rounded-lg p-4">
            <div className="flex items-center justify-between mb-2">
              <h3 className="text-lg font-semibold">{feat.name}</h3>
              <div className="flex gap-2">
                <span className={`px-2 py-1 rounded text-xs priority-${feat.priority}`}>{feat.priority}</span>
                <span className={`px-2 py-1 rounded text-xs status-${feat.status?.replace('_', '-')}`}>{feat.status}</span>
              </div>
            </div>
            <p className="text-gray-400 text-sm mb-3">{feat.description}</p>
            <div className="flex items-center gap-4">
              <span className="text-xs text-gray-500">Implementation: {feat.implementation_percentage || 0}%</span>
              <div className="flex-1">
                <ProgressBar value={feat.implementation_percentage || 0} max={100} />
              </div>
            </div>
          </div>
        ))}
        {(!data?.features || data.features.length === 0) && (
          <div className="text-center py-8 text-gray-500">
            No features registered. Use the MCP tools to create features.
          </div>
        )}
      </div>
    </div>
  )
}

// Files Page
function FilesPage() {
  const { data, isLoading, error } = useQuery({
    queryKey: ['files'],
    queryFn: () => api('/files'),
  })

  if (isLoading) return <LoadingSpinner />
  if (error) return <ErrorMessage error={error} />

  return (
    <div className="space-y-6">
      <h2 className="text-2xl font-bold">Indexed Files</h2>
      <div className="bg-ombra-darker border border-gray-800 rounded-lg overflow-hidden">
        <table className="w-full">
          <thead className="bg-gray-800">
            <tr>
              <th className="px-4 py-3 text-left text-xs text-gray-400">File</th>
              <th className="px-4 py-3 text-left text-xs text-gray-400">LOC</th>
              <th className="px-4 py-3 text-left text-xs text-gray-400">Functions</th>
              <th className="px-4 py-3 text-left text-xs text-gray-400">Stubs</th>
            </tr>
          </thead>
          <tbody>
            {data?.files?.map((file) => (
              <tr key={file.id} className="border-t border-gray-800 hover:bg-gray-800/30">
                <td className="px-4 py-3">
                  <span className="font-mono text-sm">{file.filename}</span>
                </td>
                <td className="px-4 py-3 text-gray-400">{file.line_count}</td>
                <td className="px-4 py-3 text-gray-400">{file.function_count || 0}</td>
                <td className="px-4 py-3">
                  {file.stub_count > 0 ? (
                    <span className="text-ombra-warning">{file.stub_count}</span>
                  ) : (
                    <span className="text-gray-500">0</span>
                  )}
                </td>
              </tr>
            ))}
          </tbody>
        </table>
        {(!data?.files || data.files.length === 0) && (
          <div className="text-center py-8 text-gray-500">
            No files indexed. Run a codebase scan using MCP tools.
          </div>
        )}
      </div>
    </div>
  )
}

// Stubs Page
function StubsPage() {
  const { data, isLoading, error } = useQuery({
    queryKey: ['stubs'],
    queryFn: () => api('/stubs'),
  })

  if (isLoading) return <LoadingSpinner />
  if (error) return <ErrorMessage error={error} />

  return (
    <div className="space-y-6">
      <h2 className="text-2xl font-bold">Stub Detection</h2>
      <div className="grid gap-4">
        {data?.stubs?.map((stub) => (
          <div key={stub.id} className="bg-ombra-darker border border-ombra-warning/30 rounded-lg p-4">
            <div className="flex items-center justify-between mb-2">
              <h3 className="font-mono">{stub.function_name}</h3>
              <span className="px-2 py-1 bg-ombra-warning/20 text-ombra-warning rounded text-xs">
                {stub.stub_type}
              </span>
            </div>
            <p className="text-gray-400 text-sm">{stub.detection_reason}</p>
            <div className="mt-2 text-xs text-gray-500">
              {stub.file_path}:{stub.line_number}
            </div>
          </div>
        ))}
        {(!data?.stubs || data.stubs.length === 0) && (
          <div className="text-center py-8 text-gray-500">
            No stubs detected. Run a codebase scan to detect stubs.
          </div>
        )}
      </div>
    </div>
  )
}

// Tasks Page
function TasksPage() {
  const { data, isLoading, error } = useQuery({
    queryKey: ['tasks'],
    queryFn: () => api('/tasks'),
  })

  if (isLoading) return <LoadingSpinner />
  if (error) return <ErrorMessage error={error} />

  return (
    <div className="space-y-6">
      <h2 className="text-2xl font-bold">Tasks</h2>
      <div className="grid gap-4">
        {data?.tasks?.map((task) => (
          <div key={task.id} className="bg-ombra-darker border border-gray-800 rounded-lg p-4">
            <div className="flex items-center gap-3 mb-2">
              <span className={`px-2 py-1 rounded text-xs priority-${task.priority}`}>{task.priority}</span>
              <h3 className="font-semibold flex-1">{task.title}</h3>
              <span className={`px-2 py-1 rounded text-xs status-${task.status?.replace('_', '-')}`}>
                {task.status}
              </span>
            </div>
            <p className="text-gray-400 text-sm">{task.description}</p>
          </div>
        ))}
        {(!data?.tasks || data.tasks.length === 0) && (
          <div className="text-center py-8 text-gray-500">
            No tasks created. Use MCP tools to create tasks.
          </div>
        )}
      </div>
    </div>
  )
}

// Search Page
function SearchPage() {
  const [query, setQuery] = useState('')
  const [results, setResults] = useState(null)

  const handleSearch = async () => {
    if (!query.trim()) return
    try {
      const res = await api(`/functions/search?query=${encodeURIComponent(query)}`)
      setResults(res)
    } catch (err) {
      console.error(err)
    }
  }

  return (
    <div className="space-y-6">
      <h2 className="text-2xl font-bold">Search</h2>
      <div className="flex gap-4">
        <input
          type="text"
          value={query}
          onChange={(e) => setQuery(e.target.value)}
          onKeyDown={(e) => e.key === 'Enter' && handleSearch()}
          placeholder="Search functions, files, features..."
          className="flex-1 px-4 py-2 bg-gray-800 border border-gray-700 rounded-lg focus:border-ombra-accent focus:outline-none"
        />
        <button
          onClick={handleSearch}
          className="px-6 py-2 bg-ombra-accent text-black font-semibold rounded-lg hover:bg-ombra-accent/80"
        >
          Search
        </button>
      </div>
      {results && (
        <div className="bg-ombra-darker border border-gray-800 rounded-lg p-4">
          <h3 className="text-lg font-semibold mb-4">Results</h3>
          <pre className="text-sm text-gray-300 overflow-auto">
            {JSON.stringify(results, null, 2)}
          </pre>
        </div>
      )}
    </div>
  )
}

// Loading Spinner
function LoadingSpinner() {
  return (
    <div className="flex items-center justify-center h-64">
      <div className="animate-spin rounded-full h-8 w-8 border-2 border-ombra-accent border-t-transparent" />
    </div>
  )
}

// Error Message
function ErrorMessage({ error }) {
  return (
    <div className="bg-red-900/20 border border-red-800 rounded-lg p-4 text-red-400">
      Error: {error.message}
    </div>
  )
}

// Main App
export default function App() {
  return (
    <div className="flex h-screen bg-ombra-dark">
      <Sidebar />
      <main className="flex-1 overflow-auto p-6">
        <Routes>
          <Route path="/" element={<Dashboard />} />
          <Route path="/components" element={<ComponentsPage />} />
          <Route path="/features" element={<FeaturesPage />} />
          <Route path="/files" element={<FilesPage />} />
          <Route path="/stubs" element={<StubsPage />} />
          <Route path="/tasks" element={<TasksPage />} />
          <Route path="/search" element={<SearchPage />} />
        </Routes>
      </main>
    </div>
  )
}
